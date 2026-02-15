import { addHandler } from '../add.js';

describe('Add Tool', () => {
  it('should add two positive numbers correctly', async () => {
    const result = await addHandler({ a: '5', b: '3' });
    expect(result.content[0].text).toBe('5 + 3 = 8');
  });

  it('should add negative numbers correctly', async () => {
    const result = await addHandler({ a: '-5', b: '3' });
    expect(result.content[0].text).toBe('-5 + 3 = -2');
  });

  it('should handle decimal numbers', async () => {
    const result = await addHandler({ a: '2.5', b: '1.5' });
    expect(result.content[0].text).toBe('2.5 + 1.5 = 4');
  });

  it('should handle zero', async () => {
    const result = await addHandler({ a: '0', b: '5' });
    expect(result.content[0].text).toBe('0 + 5 = 5');
  });

  it('should return error for invalid input', async () => {
    const result = await addHandler({ a: 'abc', b: '3' });
    expect(result.content[0].text).toBe('Error: Invalid numbers provided');
  });

  it('should return error when both inputs are invalid', async () => {
    const result = await addHandler({ a: 'abc', b: 'xyz' });
    expect(result.content[0].text).toBe('Error: Invalid numbers provided');
  });
});
