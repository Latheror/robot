import { createRequestHandler } from '../index.js';
import * as http from 'http';
import request from 'supertest';

describe('MCP Server HTTP Handler', () => {
  let server: http.Server;

  beforeAll((done) => {
    const requestHandler = createRequestHandler();
    server = http.createServer(requestHandler);
    server.listen(0, done); // Listen on random port
  });

  afterAll((done) => {
    server.close(done);
  });

  it('should respond to health endpoint with OK status', async () => {
    const response = await request(server).get('/health');
    expect(response.status).toBe(200);
    expect(response.headers['content-type']).toMatch(/application\/json/);
    expect(response.body).toEqual({ status: 'ok', version: '1.0.0' });
  });

  it('should handle CORS preflight OPTIONS request', async () => {
    const response = await request(server).options('/');
    expect(response.status).toBe(200);
    expect(response.headers['access-control-allow-origin']).toBe('*');
    expect(response.headers['access-control-allow-methods']).toBe('GET, POST, OPTIONS');
    expect(response.headers['access-control-allow-headers']).toBe(
      'Content-Type, mcp-session-id, Last-Event-ID, mcp-protocol-version'
    );
  });

  it('should set CORS headers on all responses', async () => {
    const response = await request(server).get('/health');
    expect(response.headers['access-control-allow-origin']).toBe('*');
    expect(response.headers['access-control-allow-methods']).toBe('GET, POST, OPTIONS');
    expect(response.headers['access-control-allow-headers']).toBe(
      'Content-Type, mcp-session-id, Last-Event-ID, mcp-protocol-version'
    );
  });

  it('should return 404 for unknown routes', async () => {
    const response = await request(server).get('/unknown');
    expect(response.status).toBe(404);
    expect(response.text).toBe('Not Found');
  });

  it('should handle MCP GET request to root without crashing', async () => {
    const response = await request(server).get('/');
    // Since MCP requires proper protocol, it might return an error or empty, but shouldn't 404
    expect(response.status).not.toBe(404);
    // It should be JSON or have proper headers
    expect(response.headers['content-type']).toMatch(/application\/json/);
  });

  it('should handle MCP POST request to root without crashing', async () => {
    const response = await request(server).post('/').send({});
    expect(response.status).not.toBe(404);
    expect(response.headers['content-type']).toMatch(/application\/json/);
  });

  // Note: Full MCP protocol testing would require mocking the transport or using MCP client,
  // but this tests that the endpoint exists and handles requests without server errors.
});
