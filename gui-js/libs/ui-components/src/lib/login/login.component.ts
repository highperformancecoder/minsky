import { Component, OnInit, OnDestroy, ElementRef, ViewChild } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MatButtonModule } from '@angular/material/button';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';
import { ClerkService } from '@minsky/core';
import { ElectronService } from '@minsky/core';
import { ActivatedRoute } from '@angular/router';
import { take } from 'rxjs';

@Component({
  selector: 'minsky-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss'],
  standalone: true,
  imports: [
    CommonModule,
    MatButtonModule,
    MatProgressSpinnerModule,
  ],
})
export class LoginComponent implements OnInit, OnDestroy {
  // Use a setter so mountClerkUI() fires as soon as *ngIf renders the div,
  // regardless of whether initializeSession has already finished or not.
  @ViewChild('clerkSignIn')
  set clerkSignInEl(el: ElementRef<HTMLDivElement>) {
    if (el && this.pendingMount) {
      this.pendingMount = false;
      this._clerkSignInEl = el;
      this.mountClerkUI();
    } else {
      this._clerkSignInEl = el;
    }
  }
  get clerkSignInEl(): ElementRef<HTMLDivElement> {
    return this._clerkSignInEl;
  }
  private _clerkSignInEl: ElementRef<HTMLDivElement>;

  isLoading = true;
  errorMessage = '';
  isAuthenticated = false;

  private unsubscribeClerk: (() => void) | null = null;
  private pendingMount = false;

  constructor(
    private clerkService: ClerkService,
    private electronService: ElectronService,
    private route: ActivatedRoute
  ) {}

  async ngOnInit() {
    this.route.queryParams.pipe(take(1)).subscribe((params) => {
      this.initializeSession(params['authToken']);
    });
  }

  ngOnDestroy() {
    this.unsubscribeClerk?.();
  }

  private async initializeSession(authToken: string | undefined) {
    // Keep initialize() errors separate: if Clerk itself fails to load we
    // cannot show the sign-in UI and must bail out early.
    try {
      await this.clerkService.initialize();
    } catch (err) {
      this.errorMessage = 'Authentication service failed to load. Please restart the application.';
      this.isLoading = false;
      return;
    }

    try {
      if (authToken) {
        await this.clerkService.setSession(authToken);
      }

      this.isAuthenticated = await this.clerkService.isSignedIn();

      if (!this.isAuthenticated) {
        this.scheduleOrMountUI();
      }
    } catch (err) {
      this.errorMessage = 'Session expired. Please sign in again.';
      this.isAuthenticated = false;
      this.scheduleOrMountUI();
    } finally {
      this.isLoading = false;
    }
  }

  private scheduleOrMountUI() {
    if (this._clerkSignInEl) {
      this.mountClerkUI();
    } else {
      this.pendingMount = true;
    }
  }

  private mountClerkUI() {
    try {
      this.clerkService.mountSignIn(this._clerkSignInEl.nativeElement);
    } catch (err: any) {
      this.errorMessage = err?.message ?? 'Failed to load sign-in UI.';
      return;
    }
    this.unsubscribeClerk = this.clerkService.addListener(async ({ session }) => {
      if (session) {
        await this.clerkService.sendTokenToElectron();
        this.electronService.closeWindow();
      }
    });
  }

  async onSignOut() {
    this.isLoading = true;
    this.errorMessage = '';
    try {
      await this.clerkService.signOut();
      this.isAuthenticated = false;
    } catch (err: any) {
      this.errorMessage = err?.message ?? 'Sign out failed.';
    } finally {
      this.isLoading = false;
    }
    this.electronService.closeWindow();
  }
}

