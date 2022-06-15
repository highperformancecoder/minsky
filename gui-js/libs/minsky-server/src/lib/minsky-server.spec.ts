import { minskyServer } from './minsky-server';

describe('minskyServer', () => {
  it('should work', () => {
    expect(minskyServer()).toEqual('minsky-server');
  });
});
