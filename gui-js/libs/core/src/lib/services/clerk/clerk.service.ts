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
    // standardBrowser: true forces the UI component renderer to initialise even
    // in non-standard browser contexts such as Electron (where environment
    // detection may otherwise return false and leave #componentControls null,
    // causing mountSignIn() to throw "Clerk was not loaded with Ui components").
    await this.clerk.load({ standardBrowser: true });
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

  mountSignIn(element: HTMLElement): void {
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
