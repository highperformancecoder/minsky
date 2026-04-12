// The file contents for the current environment will overwrite these during build.
// The build system defaults to the dev environment which uses `index.ts`, but if you do
// `ng build --env=prod` then `index.prod.ts` will be used instead.
// The list of which env maps to which file can be found in `.angular-cli.json`.

export const AppConfig = {
  production: false,
  environment: 'DEV',
  clerkPublishableKey: 'pk_test_cG9zaXRpdmUtcGhvZW5peC04NS5jbGVyay5hY2NvdW50cy5kZXYk',
  clerkOAuthRedirectUrl: 'https://clerk.minsky.app/v1/oauth_callback',
};
