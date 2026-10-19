import { Injectable } from '@angular/core';
import { ElectronService } from '../electron/electron.service';
import { events } from '@minsky/shared';
import { Clerk } from '@clerk/clerk-js';
import { AppConfig } from '@minsky/environment';

@Injectable({
  providedIn: 'root',
})
export class ClerkService {
  private clerk: Clerk;
  private initialized = false;

  constructor(private electronService: ElectronService) {}

  async initialize(): Promise<void> {
    if (this.initialized) return;

    this.clerk = new Clerk(AppConfig.clerkPublishableKey);
    if (this.electronService.isElectron) {
      this.bridgeNativeRequests();
    }
    await this.clerk.load(
      this.electronService.isElectron ? { __internal_oauthTransport: this.buildOAuthTransport() } as any : undefined
    );
    this.initialized = true;
  }

  // Tells clerk-js to hand OAuth/SSO flows to us instead of doing a same-document
  // redirect (which would navigate the whole Angular app away). clerk-js calls `open()`
  // with the provider's verification URL and awaits the callback URL our popup captures,
  // then completes the sign-in itself using that URL — this is the same extension point
  // @clerk/electron's renderer SDK uses internally (see ClerkService.bridgeNativeRequests).
  private buildOAuthTransport() {
    return {
      getRedirectUrl: () => 'http://localhost/oauth-callback',
      open: async (url: URL) => ({
        callbackUrl: await this.electronService.invoke(events.OAUTH_OPEN_POPUP, url.href),
      }),
    };
  }

  // Clerk's Frontend API identifies the requesting browser via a `__client` cookie
  // (SameSite=Lax), which only survives cross-origin requests when the app and the
  // Frontend API share a top-level site (e.g. a web app on a *.ravelation.net satellite
  // domain). Electron's renderer runs on http://localhost, an unrelated site, so the
  // cookie is dropped on every request after the first — each subsequent call looks like
  // a brand-new, anonymous client, surfacing as "signed_out"/"authorization_invalid"
  // errors partway through multi-step flows (2FA, OAuth, password reset). Clerk's own
  // @clerk/electron SDK works around this by carrying the client identity in an
  // Authorization header instead; we replicate that here rather than pulling in
  // @clerk/electron's React-only renderer SDK.
  private bridgeNativeRequests(): void {
    this.clerk.__internal_onBeforeRequest(async (request) => {
      request.credentials = 'omit';
      const token = await this.electronService.invoke(events.GET_CLERK_CLIENT_JWT);
      if (token) {
        const headers = new Headers(request.headers);
        headers.set('Authorization', `Bearer ${token}`);
        request.headers = headers;
      }
    });
    this.clerk.__internal_onAfterResponse(async (_request, response) => {
      const authorization = response?.headers?.get('Authorization');
      if (!authorization) return;
      const token = authorization.startsWith('Bearer ') ? authorization.slice(7) : authorization;
      await this.electronService.invoke(events.SET_CLERK_CLIENT_JWT, token);
    });
  }

  async isSignedIn(): Promise<boolean> {
    if (!this.clerk) return false;
    return !!this.clerk.user;
  }

  async getToken(): Promise<string | null> {
    if (!this.clerk?.session) return null;
    return await this.clerk.session.getToken();
  }

  async signInWithEmailPassword(email: string | null | undefined, password: string | null | undefined): Promise<'complete' | 'needs_second_factor'> {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    if (!email || !password) throw new Error('Email and password are required.');
    const result = await this.clerk.client.signIn.create({
      identifier: email,
      password,
    });
    return this.finalizeSignInAttempt(result);
  }

  async signInWithOAuth(strategy: string): Promise<'complete' | 'needs_second_factor'> {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    await this.clerk.client.signIn.authenticateWithRedirect({
      strategy: strategy as any,
      redirectUrl: 'http://localhost/oauth-callback',
      redirectUrlComplete: 'http://localhost/oauth-callback',
    });
    // On success, clerk-js's internal redirect-callback handling already calls
    // setActive() for us, which resets `client.signIn` back to a blank attempt —
    // re-checking its status here would look like a fresh, incomplete sign-in even
    // though we're already signed in. Check for the resulting session first.
    if (this.clerk.session) {
      await this.sendTokenToElectron();
      return 'complete';
    }
    return this.finalizeSignInAttempt(this.clerk.client.signIn);
  }

  private async finalizeSignInAttempt(result: {
    status: string;
    createdSessionId?: string | null;
    supportedSecondFactors?: { strategy: string; emailAddressId?: string }[] | null;
  }): Promise<'complete' | 'needs_second_factor'> {
    if (result.status === 'complete') {
      await this.clerk.setActive({ session: result.createdSessionId });
      await this.sendTokenToElectron();
      return 'complete';
    }
    if (result.status === 'needs_second_factor') {
      // Only email-code second factors are supported by this login form.
      const emailFactor = result.supportedSecondFactors?.find((f) => f.strategy === 'email_code');
      if (!emailFactor) throw new Error('This account requires a second factor that is not supported here.');
      await this.clerk.client.signIn.prepareSecondFactor({ strategy: 'email_code', emailAddressId: emailFactor.emailAddressId });
      return 'needs_second_factor';
    }
    throw new Error('Sign-in was not completed. Additional steps may be required.');
  }

  async attemptSecondFactorEmailCode(code: string): Promise<void> {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    const result = await this.clerk.client.signIn.attemptSecondFactor({ strategy: 'email_code', code });
    if (result.status !== 'complete') {
      throw new Error('Verification failed. Please check the code and try again.');
    }
    await this.clerk.setActive({ session: result.createdSessionId });
    await this.sendTokenToElectron();
  }

  async signOut(): Promise<void> {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    await this.clerk.signOut();
    if (this.electronService.isElectron) {
      await this.electronService.invoke(events.SET_AUTH_TOKEN, null);
    }
  }

  getSupportedOAuthStrategies(): string[] {
    if (!this.clerk) return [];
    // Try Clerk's internal environment (may vary across SDK versions)
    const env = (this.clerk as any).__unstable__environment;
    if (env?.userSettings?.social) {
      const social = env.userSettings.social as Record<string, { enabled: boolean }>;
      const enabled = Object.entries(social)
        .filter(([, s]) => s.enabled)
        .map(([name]) => `oauth_${name}`);
      if (enabled.length > 0) return enabled;
    }
    // Fallback: common providers
    // TODO: enable Apple and Microsoft OAuth providers
    //return ['oauth_github', 'oauth_google', 'oauth_apple', 'oauth_microsoft'];
    return ['oauth_github', 'oauth_google'];
  }

  async sendTokenToElectron(): Promise<void> {
    if (!this.electronService.isElectron) return;
    const token = await this.getToken();
    await this.electronService.invoke(events.SET_AUTH_TOKEN, token);
  }

  async setSession(token: string): Promise<void> {
    if (!this.clerk) throw new Error('Clerk not initialized');
    // clerk.load() in initialize() restores the session from browser storage if still valid.
    // If no session is active but sessions exist on the client, activate the first available one.
    // The token parameter is a hint that the user previously authenticated.
    if (!token) return;
    if (!this.clerk.session && this.clerk.client?.sessions?.length > 0) {
      await this.clerk.setActive({ session: this.clerk.client.sessions[0].id });
    }
    if (!this.clerk.session) {
      if (this.electronService.isElectron) 
        await this.electronService.invoke(events.SET_AUTH_TOKEN, null);
      throw new Error('Session expired or invalid');
    }
  }
}
