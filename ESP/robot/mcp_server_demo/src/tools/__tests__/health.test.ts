import { healthHandler } from '../health.js';

describe('Health Tool', () => {
  it('should return server is healthy message', () => {
    const result = healthHandler();
    expect(result.content[0].text).toBe('Server is healthy!');
  });
});
