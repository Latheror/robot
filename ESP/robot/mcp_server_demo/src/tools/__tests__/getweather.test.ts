import { weatherHandler } from '../getweather.js';

describe('Weather Tool', () => {
  it('should return sunny weather', () => {
    const result = weatherHandler();
    expect(result.content[0].text).toBe('sunny');
  });
});
