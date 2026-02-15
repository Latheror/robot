import { McpServer } from '@modelcontextprotocol/sdk/server/mcp.js';
import { z } from 'zod';

const textResponse = (text: string) => ({
  content: [{ type: 'text' as const, text }],
});

export function weatherHandler() {
  return textResponse('sunny');
}

export function registerWeatherTool(server: McpServer) {
  server.registerTool(
    'getweather',
    {
      description: 'Get the current weather',
      inputSchema: z.object({}),
    },
    weatherHandler
  );
}
