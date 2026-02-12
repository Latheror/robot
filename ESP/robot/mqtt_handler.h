#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>          // Needed for File type
#include <LittleFS.h>    // File system for audio storage
#include <functional>

/**
 * @class MqttHandler
 * @brief Handles MQTT communication for the robot, including setup, message handling, 
 *        command execution, and audio streaming playback.
 */
class MqttHandler {
public:
    /**
     * @brief Constructs the MQTT handler and initializes internal components.
     */
    MqttHandler();

    /**
     * @brief Initializes the MQTT client, sets up server details, and connects to the broker.
     *        Requires WiFi to be connected beforehand (managed by WiFiManager).
     * @return True if the connection and subscriptions succeed, false otherwise.
     */
    bool setup();

    /**
     * @brief Handles MQTT events and maintains connection.
     * 
     * This should be called frequently inside the main loop or FreeRTOS task.
     * It attempts reconnection if the client gets disconnected.
     */
    void handle();

    /**
     * @brief Publishes a message to a specific MQTT topic.
     * @param topic The topic string (e.g., "robot/1/status").
     * @param message The message payload as a string.
     * @return True if the message was successfully published.
     */
    bool publishMessage(const char* topic, const char* message);

    /**
     * @brief Registers a callback function that will be called after a command is received and executed.
     * @param callback A function to be called when a new command message is processed.
     */
    void setOnCommandReceivedCallback(std::function<void()> callback);

    // --- MQTT Topics accessible outside ---
    static constexpr const char* MQTT_TOPIC_COMMANDS = "robot/1/commands";  ///< Topic for receiving robot commands
    static constexpr const char* MQTT_TOPIC_AUDIO    = "robot/1/audio";     ///< Topic for receiving audio chunks
    static constexpr const char* MQTT_TOPIC_SENSORS  = "robot/1/sensors";   ///< Topic for publishing sensor data
    static constexpr const char* MQTT_TOPIC_FACE     = "robot/1/face";      ///< Topic for setting face expressions

private:
    /// Internal WiFi client used by PubSubClient.
    WiFiClient wifiClient;

    /// The PubSubClient instance responsible for handling MQTT communication.
    PubSubClient mqttClient{wifiClient};

    /// Callback function invoked when a command message has been received and processed.
    std::function<void()> onCommandReceivedCallback;

    /**
     * @struct AudioState
     * @brief Tracks state information for incoming audio data chunks.
     */
    struct AudioState {
        int expectedChunks = 0;  ///< Total number of audio chunks expected.
        int lastChunkIndex = -1; ///< Index of the last received chunk.
        File audioFile;          ///< File handle for writing audio data.
    };

    /// Current audio processing state.
    AudioState audioState;

    /// Temporary file path for storing received audio before playback.
    static constexpr const char* TEMP_AUDIO_FILE = "/temp_audio.wav";

    /**
     * @brief Attempts to reconnect to the MQTT broker if the connection is lost.
     * @return True if reconnection succeeds, false otherwise.
     */
    bool reconnect();

    /**
     * @brief Handles incoming "command" messages from the MQTT broker.
     * 
     * Parses the JSON payload, controls servos, and triggers feedback sounds.
     * @param message The JSON message received on the command topic.
     */
    void handleCommand(const char* message);

    /**
     * @brief Handles incoming "audio" messages from the MQTT broker.
     * 
     * Decodes base64-encoded WAV chunks and writes them to LittleFS,
     * then plays the audio once all chunks are received.
     * @param message The JSON message containing the audio chunk data.
     */
    void handleAudio(const char* message);

    /**
     * @brief Handles incoming "face" messages from the MQTT broker.
     *
     * Parses the JSON payload and sets the robot's face expression.
     * @param message The JSON message containing the face expression data.
     */
    void handleFaceSetMessage(const char* message);
};

#endif // MQTT_HANDLER_H
