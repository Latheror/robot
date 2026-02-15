import { McpServer } from '@modelcontextprotocol/sdk/server/mcp.js';
import { z } from 'zod';

const textResponse = (text: string) => ({
  content: [{ type: 'text' as const, text }],
});

export function healthHandler() {
  return textResponse('Server is healthy!');
}

export function registerHealthTool(server: McpServer) {
  server.registerTool(
    'health',
    {
      description: 'Check server health',
      inputSchema: z.object({}),
    },
    healthHandler
  );
}
