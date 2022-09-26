module.exports = {
  projects: [
    '<rootDir>/apps/minsky-electron',
    '<rootDir>/apps/minsky-server',
    '<rootDir>/libs/menu',
    '<rootDir>/libs/core',
    '<rootDir>/libs/shared',
    '<rootDir>/libs/minsky-server',
    '<rootDir>/libs/ui-components',
  ],
  collectCoverage: true,
  testMatch: [ "**/__tests__/**/*.ts?(x)", "**/?(*.)+(spec|test).ts?(x)" ],
};

//     '<rootDir>/apps/minsky-web',

import type {Config} from 'jest';

const config: Config = {
  verbose: true,
};

export default config;
