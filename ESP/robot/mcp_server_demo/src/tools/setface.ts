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

    // Await the publish to confirm delivery
    await new Promise<void>((resolve, reject) => {
      client.publish(MQTT_TOPIC_FACE, jsonMessage, { qos: 1 }, (error) => {
        if (error) {
          reject(error);
        } else {
          resolve();
        }
      });
    });

    console.log('[MCP] Published face message:', jsonMessage);
    return textResponse(`Successfully sent face command: ${jsonMessage}`);
  } catch (error) {
    console.error('[MCP] Error in setFaceHandler:', error);
    return textResponse(`Error: ${error instanceof Error ? error.message : 'Unknown error'}`);
  }
}

export function registerSetFaceTool(server: McpServer) {
  server.registerTool(
    'set_robot_face',
    {
      description: `Control the robot's face expression and eye behavior via MQTT.

Use this tool whenever the user asks the robot to change its facial expression,
emotion, eye direction, or facial animation.

You can combine multiple parameters in one command.

Rules:
- "mood" sets the base facial expression.
- "animation" plays a temporary animation on top of the mood.
- "position" controls eye direction.
- If animation is used, mood remains active after animation ends.
- If no parameters are provided, do not call this tool.

Examples:
- "Make the robot happy" → mood: happy
- "Look left" → position: w
- "Laugh" → animation: laugh
- "Happy and looking up-right" → mood: happy, position: ne`,
      inputSchema: z
        .object({
          mood: MoodEnum.optional().describe(`Base facial emotion.

happy   → smiling expression
tired   → droopy eyes / low energy
angry   → frowning expression
default → neutral face

Only one mood can be active at a time.`),
          position: PositionEnum.optional().describe(`Eye direction.

n  → up
ne → up-right
e  → right
se → down-right
s  → down
sw → down-left
w  → left
nw → up-left
default → center`),
          animation: AnimationEnum.optional().describe(`Temporary facial animation.

blink     → eyes blink once
laugh     → laughing motion
confused  → puzzled expression

Animations play once and do not replace the current mood.`),
          curiosity: z.boolean().optional().describe('Adds curious eye movement overlay'),
          sweat: z.boolean().optional().describe('Adds sweat drop effect'),
        })
        .refine(
          (data) =>
            data.mood ||
            data.position ||
            data.animation ||
            data.curiosity !== undefined ||
            data.sweat !== undefined,
          { message: 'At least one parameter must be provided.' }
        ),
    },
    setFaceHandler
  );
}
