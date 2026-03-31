import { Injectable } from '@angular/core';
import { ElectronService } from '../electron/electron.service';
import { events } from '@minsky/shared';
import { Clerk } from '@clerk/clerk-js';

@Injectable({
  providedIn: 'root',
})
export class ClerkService {
  private clerk: any = null;
  private initialized = false;

  constructor(private electronService: ElectronService) {}

  async initialize(): Promise<void> {
    if (this.initialized) return;

    const publishableKey = (window as any).__clerkPublishableKey
      ?? (typeof process !== 'undefined' && process.env?.['CLERK_PUBLISHABLE_KEY'])
      ?? '';

    if (!publishableKey) {
      console.warn(
        'ClerkService: No publishable key found in window.__clerkPublishableKey or ' +
        'CLERK_PUBLISHABLE_KEY environment variable. Authentication will not be available.'
      );
      return;
    }

    this.clerk = new Clerk(publishableKey);
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
}
