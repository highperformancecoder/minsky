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

  async getOAuthRedirectUrl(strategy: 'oauth_github' | 'oauth_google' | 'oauth_apple'): Promise<string> {
    if (!this.clerk) throw new Error('Clerk not initialized');
    const result = await this.clerk.client.signIn.create({
      strategy,
      redirectUrl: 'minsky://oauth-callback',
      actionCompleteRedirectUrl: 'minsky://oauth-callback',
    });
    // Clerk v6's TypeScript types don't expose firstFactorVerification directly on the SignIn
    // return type, but it is present at runtime. We cast to any to access this internal property.
    const redirectUrl = (result as any).firstFactorVerification?.externalVerificationRedirectURL?.href
      ?? (result as any).verifications?.externalAccount?.redirectUrl;
    if (!redirectUrl) throw new Error('OAuth redirect URL not available from Clerk');
    return redirectUrl;
  }

  async handleOAuthCallback(callbackUrl: string): Promise<void> {
    if (!this.clerk) throw new Error('Clerk not initialized');
    // Point window.location so Clerk's URL parser picks up the callback params,
    // then call handleRedirectCallback which reads the pending sign-in from localStorage
    const url = new URL(callbackUrl);
    window.history.replaceState({}, '', url.pathname + url.search + url.hash);
    // handleRedirectCallback is a public Clerk method but is not in the @clerk/clerk-js
    // TypeScript type definitions in v6. Cast to any is required to call it.
    await (this.clerk as any).handleRedirectCallback();
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
