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
    await this.clerk.load();
    this.initialized = true;
  }

  async isSignedIn(): Promise<boolean> {
    if (!this.clerk) return false;
    return !!this.clerk.user;
  }

  async getToken(): Promise<string | null> {
    if (!this.clerk?.session) return null;
    return await this.clerk.session.getToken();
  }

  async signInWithEmailPassword(email: string | null | undefined, password: string | null | undefined): Promise<void> {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    if (!email || !password) throw new Error('Email and password are required.');
    const result = await this.clerk.client.signIn.create({
      identifier: email,
      password,
    });
    if (result.status === 'complete') {
      await this.clerk.setActive({ session: result.createdSessionId });
      await this.sendTokenToElectron();
    } else {
      throw new Error('Sign-in was not completed. Additional steps may be required.');
    }
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
    return ['oauth_github', 'oauth_google', 'oauth_apple', 'oauth_microsoft'];
  }

  async getOAuthRedirectUrl(strategy: string): Promise<string> {
    if (!this.clerk) throw new Error('Clerk not initialized');
    const result = await this.clerk.client.signIn.create({
      strategy: strategy as any,
      // Use a localhost URL — Clerk's SDK only accepts http/https schemes.
      // The Electron main process intercepts this navigation via will-navigate/will-redirect
      // on the popup window's webContents before any request is actually made to localhost.
      redirectUrl: 'http://localhost/oauth-callback',
      actionCompleteRedirectUrl: 'http://localhost/oauth-callback',
    });
    // Clerk v6's TypeScript types don't expose firstFactorVerification directly on the SignIn
    // return type, but it is present at runtime. We cast to any to access this internal property.
    const redirectUrl = (result as any).firstFactorVerification?.externalVerificationRedirectURL?.href
      ?? (result as any).verifications?.externalAccount?.redirectUrl;
    if (!redirectUrl) throw new Error('OAuth redirect URL not available from Clerk');
    return redirectUrl;
  }

  async handleOAuthCallback(_callbackUrl: string): Promise<void> {
    if (!this.clerk) throw new Error('Clerk not initialized');
    // After the OAuth popup completes, Clerk's servers mark the sign-in as 'complete' and
    // create a new session. The local SignIn object (from getOAuthRedirectUrl → signIn.create())
    // still holds the old 'needs_external_account' status.
    //
    // Clerk's _handleRedirectCallback reads this.client.signIn.status — NOT window.location.
    // Passing { reloadResource: 'signIn' } causes it to call signIn.reload() first, which
    // fetches the current completed state from Clerk's servers. It then sees status === 'complete'
    // and calls setActive() with the new session ID.
    //
    // In Electron, window.opener is always null for the login window, so the reloadResource
    // branch is taken. A no-op customNavigate is passed as the second argument to suppress
    // Clerk's default post-callback page navigation (we close the window ourselves).
    await (this.clerk as any).handleRedirectCallback(
      { reloadResource: 'signIn' },
      async (_to: string) => { /* suppress post-callback navigation */ },
    );
    // Belt-and-suspenders: if handleRedirectCallback set the session via setActive(),
    // clerk.session is now populated. If it's still null but sessions exist (edge case),
    // activate the first available one.
    if (!this.clerk.session && this.clerk.client?.sessions?.length > 0) {
      await this.clerk.setActive({ session: this.clerk.client.sessions[0].id });
    }
    if (!this.clerk.session) {
      throw new Error('OAuth sign-in completed but no active session was established.');
    }
    await this.sendTokenToElectron();
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
