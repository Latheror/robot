# mcp_server_demo

A basic MCP (Model Context Protocol) server implemented in TypeScript using the MCP SDK.

## Features

- MCP server with three tools:
  - `add` - adds two numbers
  - `health` - check server health status
  - `getweather` - get current weather
- Supports HTTP transport with JSON response mode
- HTTP server runs on port 1303, accessible for integration with tools like n8n
- Modular tool architecture (each tool in a separate file)
- Comprehensive test suite with Jest (unit and integration tests)
- Code linting with ESLint and formatting with Prettier
- Docker support for containerized deployment
- GitHub Actions CI for automated testing

## Files and Their Use

- `src/index.ts`: Main server implementation with HTTP handler and tool registration
- `src/__tests__/index.test.ts`: Integration tests for the HTTP server handler
- `src/tools/add.ts`: Add tool implementation
- `src/tools/health.ts`: Health check tool
- `src/tools/getweather.ts`: Weather tool
- `src/tools/__tests__/`: Unit tests for each tool
- `package.json`: Node.js project configuration with scripts for build, test, lint, and format
- `tsconfig.json`: TypeScript compiler configuration
- `jest.config.cjs`: Jest testing framework configuration (CommonJS for ES module compatibility)
- `.eslintrc.json`: ESLint configuration for code linting
- `.prettierrc.json`: Prettier configuration for code formatting
- `build/`: Compiled JavaScript output
- `Dockerfile`: Multi-stage Docker build configuration
- `docker-compose.yml`: Docker Compose configuration for container management
- `.dockerignore`: Files to exclude from Docker build context
- `.github/copilot-instructions.md`: Instructions for GitHub Copilot
- `.github/workflows/ci.yml`: GitHub Actions CI configuration

## Commands to Build and Run

### Prerequisites

- Install Node.js (version 16 or higher)

### Setup

- Install dependencies: `npm install`

### Build

- Build the project: `npm run build`

### Lint and Format

- Lint code: `npm run lint`
- Fix linting issues: `npm run lint:fix`
- Format code: `npm run format`
- Check formatting: `npm run format:check`

## Continuous Integration

The project uses GitHub Actions for CI. On every push and pull request to main/master branches, the following checks run:

- Code linting with ESLint
- Code formatting check with Prettier
- TypeScript compilation
- Test execution with Jest
- Multi-Node.js version testing (18.x, 20.x)

## Testing

The project includes comprehensive unit and integration tests using Jest:

- **Tool unit tests**: Tests for addition, health check, and weather tools
- **Server integration tests**: Tests for HTTP endpoints, CORS, error handling, and MCP request handling

Tests are located in:
- `src/tools/__tests__/` for tool unit tests
- `src/__tests__/` for server integration tests

All tests use `.test.ts` suffix and achieve high code coverage.

### Run

- Run HTTP server: `npm run start` or `npm run start:http`

### VS Code Tasks

- Use Ctrl+Shift+P > Tasks: Run Task > Build MCP Server
- Use Ctrl+Shift+P > Tasks: Run Task > Run MCP Server (HTTP)
- Use Ctrl+Shift+P > Tasks: Run Task > Test MCP Server (Jest)
- Use Ctrl+Shift+P > Tasks: Run Task > Lint Code
- Use Ctrl+Shift+P > Tasks: Run Task > Fix Linting Issues
- Use Ctrl+Shift+P > Tasks: Run Task > Format Code
- Use Ctrl+Shift+P > Tasks: Run Task > Check Code Formatting
- Use Ctrl+Shift+P > Tasks: Run Task > Build Docker Image
- Use Ctrl+Shift+P > Tasks: Run Task > Run Docker Container
- Use Ctrl+Shift+P > Tasks: Run Task > Docker Compose Up

## Docker Support

The MCP server can be run in a Docker container for easier deployment and isolation.

### Prerequisites

- Install Docker and Docker Compose

### Build and Run with Docker Compose

1. Build and start the container:
   ```bash
   docker-compose up --build
   ```

2. The server will be available at `http://localhost:1303/`
3. Health check endpoint: `http://localhost:1303/health`

### Build and Run with Docker directly

1. Build the image:
   ```bash
   docker build -t mcp-server-demo .
   ```

2. Run the container:
   ```bash
   docker run -p 1303:1303 mcp-server-demo
   ```

### Docker Files

- `Dockerfile`: Multi-stage build for the MCP server
- `docker-compose.yml`: Compose configuration for easy container management
- `.dockerignore`: Files to exclude from Docker build context

## Integration with n8n

To access from n8n docker container, run the server with HTTP transport. If n8n is in Docker, ensure the host port 1303 is accessible (e.g., use host networking or port mapping).