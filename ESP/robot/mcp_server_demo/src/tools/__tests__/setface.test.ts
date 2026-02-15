import { setFaceHandler } from '../setface.js';
import { jest, beforeEach } from '@jest/globals';

// Mock MQTT
jest.mock('mqtt', () => ({
  connect: jest.fn(() => ({
    on: jest.fn(),
    publish: jest.fn((topic: string, message: string, options: any, callback?: (error?: Error) => void) => {
      // Simulate successful publish
      if (callback) callback();
    }),
  })),
}));

describe('Set Face Tool', () => {
  beforeEach(() => {
    jest.clearAllMocks();
  });

  it('should send a mood command', async () => {
    const result = await setFaceHandler({ mood: 'happy' });
    expect(result.content[0].text).toContain('Sending face command');
    expect(result.content[0].text).toContain('"mood":"happy"');
  });

  it('should send a position command', async () => {
    const result = await setFaceHandler({ position: 'n' });
    expect(result.content[0].text).toContain('Sending face command');
    expect(result.content[0].text).toContain('"position":"n"');
  });

  it('should send an animation command', async () => {
    const result = await setFaceHandler({ animation: 'blink' });
    expect(result.content[0].text).toContain('Sending face command');
    expect(result.content[0].text).toContain('"animation":"blink"');
  });

  it('should send multiple commands', async () => {
    const result = await setFaceHandler({
      mood: 'happy',
      position: 'ne',
      curiosity: true,
    });
    expect(result.content[0].text).toContain('Sending face command');
    expect(result.content[0].text).toContain('"mood":"happy"');
    expect(result.content[0].text).toContain('"position":"ne"');
    expect(result.content[0].text).toContain('"curiosity":true');
  });
});
