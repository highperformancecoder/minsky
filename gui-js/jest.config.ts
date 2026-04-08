import { getJestProjectsAsync } from '@nx/jest';

export default async () => ({
  projects: await getJestProjectsAsync(),
  // Add any global configurations here if needed
  transform: {
    '^.+\\.(ts|mjs|js|html)$': 'jest-preset-angular',
  },
});
