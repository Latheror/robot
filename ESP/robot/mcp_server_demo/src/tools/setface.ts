import { McpServer } from '@modelcontextprotocol/sdk/server/mcp.js';
import { z } from 'zod';
import * as mqtt from 'mqtt';

const MQTT_BROKER = 'mqtt://192.168.1.210:1883';
const MQTT_TOPIC_FACE = 'robot/1/face';

const textResponse = (text: string) => ({
  content: [{ type: 'text' as const, text }],
});

// Define the face message interface
interface FaceMessage {
  mood?: string;
  position?: string;
  animation?: string;
  curiosity?: boolean;
  sweat?: boolean;
  h_flicker?: { enabled: boolean; amplitude: number };
  v_flicker?: { enabled: boolean; amplitude: number };
  autoblinker?: { enabled: boolean; interval: number; variation: number };
  idle_mode?: { enabled: boolean; interval: number; variation: number };
  eyes?: {
    open?: { left: boolean; right: boolean };
    close?: { left: boolean; right: boolean };
  };
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
  mood?: string;
  position?: string;
  animation?: string;
  curiosity?: boolean;
  sweat?: boolean;
  h_flicker?: { enabled: boolean; amplitude?: number };
  v_flicker?: { enabled: boolean; amplitude?: number };
  autoblinker?: { enabled: boolean; interval?: number; variation?: number };
  idle_mode?: { enabled: boolean; interval?: number; variation?: number };
  eyes?: {
    open?: { left?: boolean; right?: boolean };
    close?: { left?: boolean; right?: boolean };
  };
}) {
  try {
    const client = getMqttClient();

    // Build the JSON message based on provided arguments
    const message: FaceMessage = {};

    if (args.mood) {
      message.mood = args.mood;
    }
    if (args.position) {
      message.position = args.position;
    }
    if (args.animation) {
      message.animation = args.animation;
    }
    if (args.curiosity !== undefined) {
      message.curiosity = args.curiosity;
    }
    if (args.sweat !== undefined) {
      message.sweat = args.sweat;
    }
    if (args.h_flicker) {
      message.h_flicker = {
        enabled: args.h_flicker.enabled,
        amplitude: args.h_flicker.amplitude || 2,
      };
    }
    if (args.v_flicker) {
      message.v_flicker = {
        enabled: args.v_flicker.enabled,
        amplitude: args.v_flicker.amplitude || 2,
      };
    }
    if (args.autoblinker) {
      message.autoblinker = {
        enabled: args.autoblinker.enabled,
        interval: args.autoblinker.interval || 3,
        variation: args.autoblinker.variation || 2,
      };
    }
    if (args.idle_mode) {
      message.idle_mode = {
        enabled: args.idle_mode.enabled,
        interval: args.idle_mode.interval || 2,
        variation: args.idle_mode.variation || 2,
      };
    }
    if (args.eyes) {
      message.eyes = {};
      if (args.eyes.open) {
        message.eyes.open = {
          left: args.eyes.open.left !== false,
          right: args.eyes.open.right !== false,
        };
      }
      if (args.eyes.close) {
        message.eyes.close = {
          left: args.eyes.close.left !== false,
          right: args.eyes.close.right !== false,
        };
      }
    }

    // Send the message
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
        'Change the robot face expression by sending MQTT commands. All parameters are optional - only specify what you want to change.',
      inputSchema: z.object({
        mood: z
          .enum(['happy', 'tired', 'angry', 'default'])
          .optional()
          .describe('Set the mood of the robot face'),
        position: z
          .enum(['n', 'ne', 'e', 'se', 's', 'sw', 'w', 'nw', 'default'])
          .optional()
          .describe('Set the eye position'),
        animation: z
          .enum(['blink', 'laugh', 'confused'])
          .optional()
          .describe('Trigger an animation'),
        curiosity: z.boolean().optional().describe('Enable or disable curiosity mode'),
        sweat: z.boolean().optional().describe('Enable or disable sweat effect'),
        h_flicker: z
          .object({
            enabled: z.boolean(),
            amplitude: z.number().min(0).max(255).optional().default(2),
          })
          .optional()
          .describe('Configure horizontal flicker'),
        v_flicker: z
          .object({
            enabled: z.boolean(),
            amplitude: z.number().min(0).max(255).optional().default(2),
          })
          .optional()
          .describe('Configure vertical flicker'),
        autoblinker: z
          .object({
            enabled: z.boolean(),
            interval: z.number().min(0).optional().default(3),
            variation: z.number().min(0).optional().default(2),
          })
          .optional()
          .describe('Configure automatic blinking'),
        idle_mode: z
          .object({
            enabled: z.boolean(),
            interval: z.number().min(0).optional().default(2),
            variation: z.number().min(0).optional().default(2),
          })
          .optional()
          .describe('Configure idle mode'),
        eyes: z
          .object({
            open: z
              .object({
                left: z.boolean().optional().default(true),
                right: z.boolean().optional().default(true),
              })
              .optional(),
            close: z
              .object({
                left: z.boolean().optional().default(true),
                right: z.boolean().optional().default(true),
              })
              .optional(),
          })
          .optional()
          .describe('Control eye opening/closing'),
      }),
    },
    setFaceHandler
  );
}
