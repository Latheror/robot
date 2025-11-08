#ifndef SERVOS_H
#define SERVOS_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <array>

/**
 * @enum Joint
 * @brief Defines logical robot joints.
 */
enum class Joint {
    ROOT = 0,
    ARM_A,
    ARM_B,
    WRIST_A,
    WRIST_B,
    GRIPPER,
    COUNT
};

/**
 * @struct ServoInfo
 * @brief Physical servo information.
 */
struct ServoInfo {
    uint8_t pwmIndex; ///< PWM channel on the board
};

/**
 * @struct JointInfo
 * @brief Logical joint configuration.
 */
struct JointInfo {
    float minAngle;               ///< Minimum angle for the joint
    float maxAngle;               ///< Maximum angle for the joint
    float speed;                  ///< Movement speed
    std::array<int8_t, 2> servos; ///< Up to 2 physical servos per joint (2nd can be mirrored)
    uint8_t servoCount;           ///< How many servos are valid
};

/**
 * @class ServoController
 * @brief Manages servo motors for robot joints using PCA9685 PWM driver.
 */
class ServoController {
public:
    /**
     * @brief Initializes the servo controller.
     * @return true if successful, false otherwise.
     */
    static bool begin();

    /**
     * @brief Updates all servo positions.
     */
    static void update();

    /**
     * @brief Checks if the controller is initialized.
     * @return true if initialized, false otherwise.
     */
    static bool isInitialized() { return _initialized; }

    /**
     * @brief Sets the target angle for a joint.
     * @param joint The joint to set.
     * @param angle The target angle in degrees.
     */
    static void setTargetAngle(Joint joint, float angle);

    /**
     * @brief Gets the current angle of a joint.
     * @param joint The joint to query.
     * @return The current angle in degrees.
     */
    static float getCurrentAngle(Joint joint);

    /**
     * @brief Checks if a joint is moving.
     * @param joint The joint to check.
     * @return true if moving, false otherwise.
     */
    static bool isMoving(Joint joint);

    /**
     * @brief Checks if a joint is at its target.
     * @param joint The joint to check.
     * @return true if at target, false otherwise.
     */
    static bool atTarget(Joint joint);

private:
    static constexpr uint16_t PWM_MIN = 150; ///< 0° pulse width
    static constexpr uint16_t PWM_MAX = 600; ///< 180° pulse width
    static constexpr uint8_t  PWM_FREQ = 50; ///< 50Hz for servos

    static Adafruit_PWMServoDriver pwm; ///< PWM driver instance

    // Physical servos (7 total)
    static std::array<ServoInfo, 7> servos; ///< Physical servo configurations

    // Logical joints (6 total)
    static std::array<JointInfo, static_cast<size_t>(Joint::COUNT)> joints; ///< Joint configurations

    // State tracking
    static std::array<float, static_cast<size_t>(Joint::COUNT)> currentAngles; ///< Current angles
    static std::array<float, static_cast<size_t>(Joint::COUNT)> targetAngles; ///< Target angles
    static std::array<float, static_cast<size_t>(Joint::COUNT)> speeds;       ///< Movement speeds
    static bool _initialized; ///< Initialization flag

    // Internal helpers
    /**
     * @brief Updates a specific joint.
     * @param joint The joint to update.
     */
    static void updateJoint(Joint joint);

    /**
     * @brief Converts angle to PWM value.
     * @param angle The angle in degrees.
     * @return The PWM value.
     */
    static uint16_t angleToPWM(float angle);

    /**
     * @brief Validates a joint index.
     * @param joint The joint to validate.
     * @return true if valid, false otherwise.
     */
    static bool isValidJoint(Joint joint);
};

#endif // SERVOS_H
