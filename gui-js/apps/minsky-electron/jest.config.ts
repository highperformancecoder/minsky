/* eslint-disable */
import { createRequire } from 'module';
const require = createRequire(import.meta.url);

export default {
  displayName: 'minsky-electron',
  preset: '../../jest.preset.js',
  setupFilesAfterEnv: ['<rootDir>/src/test-setup.ts'],
  coverageDirectory: '../../coverage/apps/minsky-electron',
  transform: {
    '^.+\\.[tj]s$': require.resolve('ts-jest'),
  },
  transformIgnorePatterns: ['node_modules/(?!.*\\.mjs$)'],
  snapshotSerializers: [
    'jest-preset-angular/build/serializers/no-ng-attributes',
    'jest-preset-angular/build/serializers/ng-snapshot',
    'jest-preset-angular/build/serializers/html-comment',
  ],
};
