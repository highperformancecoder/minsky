import { Injectable } from '@angular/core';
import { ElectronService } from '../electron/electron.service';
import { events } from '@minsky/shared';
import type { Clerk } from '@clerk/clerk-js';
import { AppConfig } from '@minsky/environment';

declare global {
  interface Window {
    Clerk?: Clerk;
    __clerk_publishable_key?: string;
  }
}

@Injectable({
  providedIn: 'root',
})
export class ClerkService {
  private clerk: Clerk;
  private initialized = false;

  constructor(private electronService: ElectronService) {}

  async initialize(): Promise<void> {
    if (this.initialized) return;

    // Load Clerk via its CDN browser bundle rather than the npm-imported module.
    // The npm dist files (clerk.js / clerk.mjs) do not bundle the ClerkUI
    // implementation, so clerk.mountSignIn() would always throw
    // "Clerk was not loaded with Ui components" when called on an instance
    // created with `new Clerk(key)` from the npm package.
    // The browser bundle served from Clerk's CDN lazily loads the UI chunks
    // (React-based pre-built components) from the same CDN origin, enabling
    // mountSignIn() to work correctly.
    await this.loadClerkBrowserBundle();
    this.clerk = window.Clerk!;
    await this.clerk.load();
    this.initialized = true;
  }

  /**
   * Dynamically injects Clerk's CDN browser bundle script into the document.
   * The CDN URL is derived from the publishable key's embedded frontendApi.
   * Returns a Promise that resolves once the script has loaded and
   * window.Clerk has been set by the bundle.
   */
  private loadClerkBrowserBundle(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      if (window.Clerk) {
        resolve();
        return;
      }

      const pk = AppConfig.clerkPublishableKey;
      // Publishable key format: pk_<type>_<base64(frontendApi + '$')>
      const encoded = pk.split('_')[2] ?? '';
      const padded = encoded + '='.repeat((4 - (encoded.length % 4)) % 4);
      let frontendApi: string;
      try {
        frontendApi = atob(padded).replace(/\$$/, '');
      } catch {
        reject(new Error('Invalid Clerk publishable key'));
        return;
      }

      const script = document.createElement('script');
      script.src = `https://${frontendApi}/npm/@clerk/clerk-js@latest/dist/clerk.browser.js`;
      script.setAttribute('data-clerk-publishable-key', pk);
      script.async = true;
      script.onload = () => resolve();
      script.onerror = () =>
        reject(new Error('Failed to load Clerk authentication service'));
      document.head.appendChild(script);
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

  mountSignIn(element: HTMLDivElement): void {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    this.clerk.mountSignIn(element);
  }

  addListener(callback: (resources: { session: { id: string } | null }) => void): () => void {
    if (!this.clerk) throw new Error('Clerk is not initialized.');
    return this.clerk.addListener(callback);
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
