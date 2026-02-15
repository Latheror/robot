// src/index.ts
import { McpServer } from '@modelcontextprotocol/sdk/server/mcp.js';
import { StreamableHTTPServerTransport } from '@modelcontextprotocol/sdk/server/streamableHttp.js';
import * as http from 'http';
import { registerAddTool } from './tools/add.js';
import { registerHealthTool } from './tools/health.js';
import { registerWeatherTool } from './tools/getweather.js';

const PORT = 1303;
const HOST = '0.0.0.0';

// Helper function to register tools on a server instance
function registerTools(server: McpServer) {
  registerAddTool(server);
  registerHealthTool(server);
  registerWeatherTool(server);
}

// Export the request handler for testing
export function createRequestHandler() {
  return async (req: http.IncomingMessage, res: http.ServerResponse) => {
    // CORS
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader(
      'Access-Control-Allow-Headers',
      'Content-Type, mcp-session-id, Last-Event-ID, mcp-protocol-version'
    );

    if (req.method === 'OPTIONS') {
      res.writeHead(200);
      res.end();
      return;
    }

    if (req.method === 'GET' && req.url === '/health') {
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ status: 'ok', version: '1.0.0' }));
      return;
    }

    if ((req.method === 'POST' || req.method === 'GET') && req.url === '/') {
      try {
        const transport = new StreamableHTTPServerTransport({
          sessionIdGenerator: undefined,
          enableJsonResponse: true,
        });

        const requestServer = new McpServer({ name: 'demo', version: '1.0.0' });
        registerTools(requestServer);

        await requestServer.connect(transport);
        await transport.handleRequest(req, res);
      } catch (error) {
        console.error('Error handling MCP request:', error);
        if (!res.headersSent) {
          res.writeHead(500, { 'Content-Type': 'application/json' });
          res.end(
            JSON.stringify({
              jsonrpc: '2.0',
              error: { code: -32603, message: 'Internal server error' },
              id: null,
            })
          );
        }
      }
      return;
    }

    res.writeHead(404).end('Not Found');
  };
}

async function main() {
  const requestHandler = createRequestHandler();
  const server_http = http.createServer(requestHandler);

  server_http.listen(PORT, HOST, () => {
    console.log(`MCP Server running on http://${HOST}:${PORT}`);
    console.log(`Health endpoint available at http://${HOST}:${PORT}/health`);
  });
}

if (process.argv[1]?.endsWith('index.js')) {
  main().catch((err) => {
    console.error('Fatal error in MCP server:', err);
    process.exit(1);
  });
}
