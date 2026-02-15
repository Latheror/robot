import { McpServer } from '@modelcontextprotocol/sdk/server/mcp.js';
import { z } from 'zod';
import * as mqtt from 'mqtt';

const MQTT_BROKER = 'mqtt://192.168.1.210:1883';
const MQTT_TOPIC_FACE = 'robot/1/face';

const textResponse = (text: string) => ({
  content: [{ type: 'text' as const, text }],
});

// Define enum values for better maintainability and sharing
export const FACE_ENUMS = {
  moods: ['happy', 'tired', 'angry', 'default'] as const,
  positions: ['n', 'ne', 'e', 'se', 's', 'sw', 'w', 'nw', 'default'] as const,
  animations: ['blink', 'laugh', 'confused'] as const,
};

// Create Zod enums
const MoodEnum = z.enum(FACE_ENUMS.moods);
const PositionEnum = z.enum(FACE_ENUMS.positions);
const AnimationEnum = z.enum(FACE_ENUMS.animations);

// Define the face message interface
interface FaceMessage {
  mood?: z.infer<typeof MoodEnum>;
  position?: z.infer<typeof PositionEnum>;
  animation?: z.infer<typeof AnimationEnum>;
  curiosity?: boolean;
  sweat?: boolean;
}

// MQTT client instance
let mqttClient: mqtt.MqttClient | null = null;

// Initialize MQTT client
function getMqttClient(): mqtt.MqttClient {
  if (!mqttClient) {
    mqttClient = mqtt.connect(MQTT_BROKER);
    mqttClient.on('connect', () => {
      console.log('[MCP] Connected to MQTT broker');
    });
    mqttClient.on('error', (error) => {
      console.error('[MCP] MQTT connection error:', error);
    });
  }
  return mqttClient;
}

export async function setFaceHandler(args: {
  mood?: z.infer<typeof MoodEnum>;
  position?: z.infer<typeof PositionEnum>;
  animation?: z.infer<typeof AnimationEnum>;
  curiosity?: boolean;
  sweat?: boolean;
}) {
  try {
    const client = getMqttClient();

    // Build the JSON message based on provided arguments
    const message = Object.fromEntries(
      Object.entries({
        mood: args.mood,
        position: args.position,
        animation: args.animation,
        curiosity: args.curiosity,
        sweat: args.sweat,
      }).filter(([, v]) => v !== undefined)
    ) as FaceMessage;

    const jsonMessage = JSON.stringify(message);
    client.publish(MQTT_TOPIC_FACE, jsonMessage, { qos: 1 }, (error) => {
      if (error) {
        console.error('[MCP] Failed to publish face message:', error);
        return textResponse(`Error: Failed to send face command - ${error.message}`);
      } else {
        console.log('[MCP] Published face message:', jsonMessage);
        return textResponse(`Successfully sent face command: ${jsonMessage}`);
      }
    });

    // Since publish is async, return a response immediately
    return textResponse(`Sending face command: ${jsonMessage}`);
  } catch (error) {
    console.error('[MCP] Error in setFaceHandler:', error);
    return textResponse(`Error: ${error instanceof Error ? error.message : 'Unknown error'}`);
  }
}

export function registerSetFaceTool(server: McpServer) {
  server.registerTool(
    'set_robot_face',
    {
      description:
        'Change the robot face expression by sending MQTT commands. Use simple parameters for basic control.',
      inputSchema: z.object({
        mood: MoodEnum.optional().describe(
          `Set the mood of the robot face. Options: ${FACE_ENUMS.moods.join(', ')}`
        ),
        position: PositionEnum.optional().describe(
          `Set the eye position. Options: ${FACE_ENUMS.positions.join(', ')}`
        ),
        animation: AnimationEnum.optional().describe(
          `Trigger an animation. Options: ${FACE_ENUMS.animations.join(', ')}`
        ),
        curiosity: z
          .boolean()
          .optional()
          .describe('Enable or disable curiosity mode.'),
        sweat: z
          .boolean()
          .optional()
          .describe('Enable or disable sweat effect.'),
      }),
    },
    setFaceHandler
  );
}
