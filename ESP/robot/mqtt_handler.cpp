#include "mqtt_handler.h"
#include "settings.h"
#include "servos.h"
#include "Speaker.h"
#include "indicators.h"
#include "roboeyes_display.h"
#include <LittleFS.h>
#include <mbedtls/base64.h>
#include <Arduino.h>

extern Speaker speaker;
extern Indicators indicators;
extern SemaphoreHandle_t audioMutex;
extern RoboEyesDisplay roboEyes;

MqttHandler::MqttHandler() {
    // mqttClient is already initialized internally with wifiClient
    audioState = {};
}

bool MqttHandler::setup() {
    Serial.println("[MQTT] Initializing...");

    mqttClient.setBufferSize(SystemConfig::MQTT_BUFFER_SIZE);
    mqttClient.setServer(NetworkConfig::MQTT_BROKER, NetworkConfig::MQTT_PORT);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        // Safety check: reject too-large messages
        if (length > SystemConfig::MQTT_MAX_MESSAGE_SIZE) {
            Serial.printf("[MQTT] Message too large (%u bytes), rejected\n", length);
            return;
        }
        
        // Allocate message buffer dynamically to avoid stack overflow
        std::unique_ptr<char[]> message(new char[length + 1]);
        if (!message) {
            Serial.println("[MQTT] Failed to allocate message buffer");
            return;
        }
        
        memcpy(message.get(), payload, length);
        message.get()[length] = '\0';
        
        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) {
            handleCommand(message.get());
        } else if (strcmp(topic, MQTT_TOPIC_AUDIO) == 0) {
            handleAudio(message.get());
        } else if (strcmp(topic, MQTT_TOPIC_FACE) == 0) {
            handleFaceSetMessage(message.get());
        }
    });

    return reconnect();
}

/**
 * @brief Attempts to reconnect to the MQTT broker.
 * @return true if successful, false otherwise.
 */
bool MqttHandler::reconnect() {
    Serial.println("[MQTT] Attempting to connect...");

    // Red for connecting
    indicators.setColor(Indicators::LED_PINS::MQTT, 255, 0, 0);

    for (int attempt = 0; attempt < 3 && !mqttClient.connected(); attempt++) {
        if (mqttClient.connect(NetworkConfig::MQTT_CLIENT_ID)) {
            mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
            mqttClient.subscribe(MQTT_TOPIC_AUDIO);
            mqttClient.subscribe(MQTT_TOPIC_FACE);
            Serial.println("[MQTT] Connected and subscribed");
            // Green for connected
            indicators.set(Indicators::LED_PINS::MQTT, true);
            return true;
        }
        delay(TaskConfig::MQTT_RECONNECT_DELAY_MS);
        Serial.println("[MQTT] Retry connecting...");
    }

    Serial.println("[MQTT] Failed to connect");
    // Turn LED off for disconnected state
    indicators.set(Indicators::LED_PINS::MQTT, false);
    return false;
}

void MqttHandler::handle() {
    static unsigned long lastReconnectAttempt = 0;

    if (!mqttClient.connected()) {
        if (millis() - lastReconnectAttempt > SystemConfig::RECONNECT_DELAY_MS) {
            lastReconnectAttempt = millis();
            reconnect();
        }
        return;
    }

    mqttClient.loop();
}

bool MqttHandler::publishMessage(const char* topic, const char* message) {
    Serial.printf("[MQTT] Publishing to %s: %s\n", topic, message);
    return mqttClient.connected() && mqttClient.publish(topic, message);
}

void MqttHandler::setOnCommandReceivedCallback(std::function<void()> callback) {
    onCommandReceivedCallback = callback;
}

// --- Command handling ---

/**
 * @brief Handles incoming MQTT commands.
 * @param message The command message.
 */
void MqttHandler::handleCommand(const char* message) {
    // Validate input
    if (!message) {
        Serial.println("[MQTT] Error: NULL message pointer in handleCommand");
        return;
    }
    
    static int lastRandNum = 0;

    switch (lastRandNum) {
        case 0: speaker.playWav("/warning.wav"); break;
        case 1: speaker.playWav("/imabitclumsy.wav"); break;
        case 2: speaker.playWav("/iwilltrysomething.wav"); break;
    }
    lastRandNum = (lastRandNum + 1) % 3;

    Serial.println("[MQTT] Received command:");
    Serial.println(message);

    StaticJsonDocument<SystemConfig::JSON_DOC_SIZE> doc;
    if (deserializeJson(doc, message)) {
        Serial.println("[MQTT] Failed to parse command JSON");
        return;
    }

    if (!doc.containsKey("servos")) return;
    JsonObject servos = doc["servos"];

    const struct { const char* name; int servo; } servoMap[] = {
        {"root", static_cast<int>(Joint::ROOT)}, 
        {"arm_a", static_cast<int>(Joint::ARM_A)},
        {"arm_b", static_cast<int>(Joint::ARM_B)},
        {"wrist_a", static_cast<int>(Joint::WRIST_A)},
        {"wrist_b", static_cast<int>(Joint::WRIST_B)},
        {"gripper", static_cast<int>(Joint::GRIPPER)}
    };

    // Set motors indicator - Purple for movement
    indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 255, 0, 255);

    if (!ServoController::isInitialized()) {
        Serial.println("[MQTT] Servo controller not initialized, ignoring command");
        indicators.set(Indicators::LED_PINS::MOTORS_MOVING, false);
        return;
    }

    for (const auto& map : servoMap) {
        if (servos.containsKey(map.name)) {
            JsonVariant angleVar = servos[map.name];
            if (!angleVar.is<float>() && !angleVar.is<int>()) {
                Serial.printf("[MQTT] Invalid angle type for joint '%s'\n", map.name);
                continue;
            }
            ServoController::setTargetAngle(static_cast<Joint>(map.servo), angleVar.as<float>());
        }
    }

    // Turn off movement LED
    indicators.set(Indicators::LED_PINS::MOTORS_MOVING, false);

    if (onCommandReceivedCallback) onCommandReceivedCallback();
}

// --- Audio handling ---

/**
 * @brief Handles incoming audio chunk messages from MQTT.
 * @param message The JSON message containing audio data.
 */
void MqttHandler::handleAudio(const char* message) {
    // Protect audioState with mutex to prevent race conditions
    if (!audioMutex) {
        Serial.println("[MQTT] Audio mutex not initialized, rejecting audio chunk");
        return;
    }
    
    if (xSemaphoreTake(audioMutex, pdMS_TO_TICKS(SystemConfig::MUTEX_TIMEOUT_MS)) != pdTRUE) {
        Serial.println("[MQTT] Failed to acquire audio mutex (timeout), audio chunk rejected");
        return;
    }
    
    // Use DynamicJsonDocument for large payloads to avoid stack overflow
    // StaticJsonDocument allocates on stack, which causes overflow with large chunks
    DynamicJsonDocument doc(SystemConfig::AUDIO_JSON_DOC_SIZE);
    if (deserializeJson(doc, message)) {
        Serial.println("[MQTT] Failed to parse audio message");
        xSemaphoreGive(audioMutex);
        return;
    }

    if (!doc.containsKey("message") || !doc.containsKey("chunk_index") || !doc.containsKey("total_chunks")) {
        Serial.println("[MQTT] Invalid audio message format");
        xSemaphoreGive(audioMutex);
        return;
    }

    int chunkIndex = doc["chunk_index"];
    int totalChunks = doc["total_chunks"];
    const char* base64Data = doc["message"];

    Serial.printf("[MQTT] Processing chunk %d/%d, base64 length: %u\n", chunkIndex, totalChunks, strlen(base64Data));

    if (chunkIndex == 0) {
        // Reset previous state if exists
        if (audioState.audioFile) {
            audioState.audioFile.close();
        }
        
        if (LittleFS.exists(TEMP_AUDIO_FILE)) LittleFS.remove(TEMP_AUDIO_FILE);

        audioState.audioFile = LittleFS.open(TEMP_AUDIO_FILE, FILE_WRITE);
        if (!audioState.audioFile) {
            Serial.println("[MQTT] Failed to open temp audio file");
            audioState = {};  // Reset state
            xSemaphoreGive(audioMutex);
            return;
        }

        audioState.expectedChunks = totalChunks;
        audioState.lastChunkIndex = -1;
        Serial.println("[MQTT] Started writing audio file...");
    }

    if (!audioState.audioFile) {
        Serial.println("[MQTT] Audio file not open! Aborting audio transfer");
        audioState = {};  // Reset state
        xSemaphoreGive(audioMutex);
        return;
    }
    
    // Validate chunk index sequence
    if (chunkIndex != audioState.lastChunkIndex + 1) {
        Serial.printf("[MQTT] Out-of-order chunk: expected %d, got %d. Resetting audio state.\n", 
                      audioState.lastChunkIndex + 1, chunkIndex);
        audioState.audioFile.close();
        audioState = {};  // Reset state
        xSemaphoreGive(audioMutex);
        return;
    }

    size_t decodedLen = 0;
    mbedtls_base64_decode(nullptr, 0, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data));

    // Check heap before allocation - reserve at least 50% free
    uint32_t freeHeap = ESP.getFreeHeap();
    if (decodedLen > (freeHeap / 2)) {
        Serial.printf("[MQTT] Insufficient heap for decoding! Need: %zu, Free: %u\n", decodedLen, freeHeap);
        xSemaphoreGive(audioMutex);
        return;
    }

    std::unique_ptr<uint8_t[]> buffer(new uint8_t[decodedLen]);
    if (!buffer) {
        Serial.println("[MQTT] Failed to allocate buffer for decoding");
        xSemaphoreGive(audioMutex);
        return;
    }

    if (mbedtls_base64_decode(buffer.get(), decodedLen, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data)) != 0) {
        Serial.println("[MQTT] Base64 decode failed");
        audioState.audioFile.close();
        audioState = {};  // Reset audio state
        xSemaphoreGive(audioMutex);
        return;
    }

    audioState.audioFile.write(buffer.get(), decodedLen);

    if (chunkIndex == totalChunks - 1) {
        audioState.audioFile.close();
        Serial.println("[MQTT] Finished writing audio file, playing...");
        xSemaphoreGive(audioMutex);  // Release before playback
        speaker.playWav(TEMP_AUDIO_FILE);
        xSemaphoreTake(audioMutex, pdMS_TO_TICKS(SystemConfig::MUTEX_TIMEOUT_MS));  // Re-acquire to reset state safely
        audioState = {};  // reset
        xSemaphoreGive(audioMutex);
    } else {
        audioState.lastChunkIndex = chunkIndex;
        xSemaphoreGive(audioMutex);  // Release mutex
    }
}

// --- Face handling ---

/**
 * @brief Handles incoming "face" messages from the MQTT broker.
 *
 * Parses the JSON payload and sets the robot's face expression.
 * Enables MQTT control mode on first message (stops automatic random changes).
 *
 * @param message The JSON message containing the face expression data.
 *
 * @note All fields are optional. Send only the properties you want to change.
 *
 * Example message with all possible fields:
 * @code{.json}
 * {
 *   "mood": "happy",           // "happy", "tired", "angry", "default"
 *   "position": "ne",          // "n", "ne", "e", "se", "s", "sw", "w", "nw", "default"
 *   "animation": "laugh",      // "blink", "laugh", "confused"
 *   "curiosity": true,         // boolean: enable/disable curiosity mode
 *   "sweat": false,            // boolean: enable/disable sweat drops
 *   "h_flicker": {             // horizontal flicker settings
 *     "enabled": true,
 *     "amplitude": 2
 *   },
 *   "v_flicker": {             // vertical flicker settings
 *     "enabled": false,
 *     "amplitude": 2
 *   },
 *   "autoblinker": {           // automatic blinking settings
 *     "enabled": true,
 *     "interval": 3,           // seconds between blinks
 *     "variation": 2           // random variation in seconds
 *   },
 *   "idle_mode": {             // automatic repositioning settings
 *     "enabled": false,
 *     "interval": 2,           // seconds between repositions
 *     "variation": 2           // random variation in seconds
 *   },
 *   "eyes": {                  // manual eye control
 *     "open": {                // open specific eyes
 *       "left": true,
 *       "right": true
 *     },
 *     "close": {               // close specific eyes
 *       "left": false,
 *       "right": false
 *     }
 *   }
 * }
 * @endcode
 */
void MqttHandler::handleFaceSetMessage(const char* message) {
    // Validate input
    if (!message) {
        Serial.println("[MQTT] Error: NULL message pointer in handleFaceSetMessage");
        return;
    }

    // Enable MQTT control on first message (stops automatic random changes)
    roboEyes.enableMqttControl();

    Serial.println("[MQTT] Received face command:");
    Serial.println(message);

    StaticJsonDocument<SystemConfig::JSON_DOC_SIZE> doc;
    if (deserializeJson(doc, message)) {
        Serial.println("[MQTT] Failed to parse face JSON");
        return;
    }

    bool success = false;

    // Handle mood
    if (doc.containsKey("mood")) {
        const char* moodStr = doc["mood"];
        Mood mood;
        if (strcmp(moodStr, "happy") == 0) {
            mood = Mood::MOOD_HAPPY;
        } else if (strcmp(moodStr, "tired") == 0) {
            mood = Mood::MOOD_TIRED;
        } else if (strcmp(moodStr, "angry") == 0) {
            mood = Mood::MOOD_ANGRY;
        } else if (strcmp(moodStr, "default") == 0) {
            mood = Mood::MOOD_DEFAULT;
        } else {
            Serial.printf("[MQTT] Unknown mood '%s'\n", moodStr);
            success = false;
        }
        if (roboEyes.setMood(mood)) {
            success = true;
        }
    }

    // Handle position
    if (doc.containsKey("position")) {
        const char* posStr = doc["position"];
        Position position;
        if (strcmp(posStr, "n") == 0) {
            position = Position::POS_N;
        } else if (strcmp(posStr, "ne") == 0) {
            position = Position::POS_NE;
        } else if (strcmp(posStr, "e") == 0) {
            position = Position::POS_E;
        } else if (strcmp(posStr, "se") == 0) {
            position = Position::POS_SE;
        } else if (strcmp(posStr, "s") == 0) {
            position = Position::POS_S;
        } else if (strcmp(posStr, "sw") == 0) {
            position = Position::POS_SW;
        } else if (strcmp(posStr, "w") == 0) {
            position = Position::POS_W;
        } else if (strcmp(posStr, "nw") == 0) {
            position = Position::POS_NW;
        } else if (strcmp(posStr, "default") == 0) {
            position = Position::POS_DEFAULT;
        } else {
            Serial.printf("[MQTT] Unknown position '%s'\n", posStr);
            success = false;
        }
        if (roboEyes.setPosition(position)) {
            success = true;
        }
    }

    // Handle animation
    if (doc.containsKey("animation")) {
        const char* animStr = doc["animation"];
        Animation animation;
        if (strcmp(animStr, "blink") == 0) {
            animation = Animation::ANIM_BLINK;
        } else if (strcmp(animStr, "laugh") == 0) {
            animation = Animation::ANIM_LAUGH;
        } else if (strcmp(animStr, "confused") == 0) {
            animation = Animation::ANIM_CONFUSED;
        } else {
            Serial.printf("[MQTT] Unknown animation '%s'\n", animStr);
            success = false;
        }
        if (roboEyes.triggerAnimation(animation)) {
            success = true;
        }
    }

    // Handle curiosity
    if (doc.containsKey("curiosity")) {
        bool enabled = doc["curiosity"];
        success = roboEyes.setCuriosity(enabled) || success;
    }

    // Handle sweat
    if (doc.containsKey("sweat")) {
        bool enabled = doc["sweat"];
        success = roboEyes.setSweat(enabled) || success;
    }

    // Handle horizontal flicker
    if (doc.containsKey("h_flicker")) {
        JsonObject flicker = doc["h_flicker"];
        bool enabled = flicker["enabled"] | false;
        uint8_t amplitude = flicker["amplitude"] | 2;
        success = roboEyes.setHFlicker(enabled, amplitude) || success;
    }

    // Handle vertical flicker
    if (doc.containsKey("v_flicker")) {
        JsonObject flicker = doc["v_flicker"];
        bool enabled = flicker["enabled"] | false;
        uint8_t amplitude = flicker["amplitude"] | 2;
        success = roboEyes.setVFlicker(enabled, amplitude) || success;
    }

    // Handle autoblinker
    if (doc.containsKey("autoblinker")) {
        JsonObject blinker = doc["autoblinker"];
        bool enabled = blinker["enabled"] | false;
        int interval = blinker["interval"] | 3;
        int variation = blinker["variation"] | 2;
        success = roboEyes.setAutoblinker(enabled, interval, variation) || success;
    }

    // Handle idle mode
    if (doc.containsKey("idle_mode")) {
        JsonObject idle = doc["idle_mode"];
        bool enabled = idle["enabled"] | false;
        int interval = idle["interval"] | 2;
        int variation = idle["variation"] | 2;
        success = roboEyes.setIdleMode(enabled, interval, variation) || success;
    }

    // Handle eye control
    if (doc.containsKey("eyes")) {
        JsonObject eyes = doc["eyes"];
        if (eyes.containsKey("open")) {
            JsonObject open = eyes["open"];
            bool left = open["left"] | true;
            bool right = open["right"] | true;
            success = roboEyes.openEyes(left, right) || success;
        }
        if (eyes.containsKey("close")) {
            JsonObject close = eyes["close"];
            bool left = close["left"] | true;
            bool right = close["right"] | true;
            success = roboEyes.closeEyes(left, right) || success;
        }
    }

    if (!success) {
        Serial.println("[MQTT] Failed to set face expression");
    }
}
