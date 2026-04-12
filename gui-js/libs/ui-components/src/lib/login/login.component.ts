import { Component, OnInit, OnDestroy, ElementRef, ViewChild, AfterViewInit } from '@angular/core';
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
export class LoginComponent implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('clerkSignIn') clerkSignInEl!: ElementRef<HTMLDivElement>;

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

  ngAfterViewInit() {
    if (this.pendingMount) {
      this.mountClerkUI();
      this.pendingMount = false;
    }
  }

  ngOnDestroy() {
    this.unsubscribeClerk?.();
  }

  private async initializeSession(authToken: string | undefined) {
    try {
      await this.clerkService.initialize();

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
    if (this.clerkSignInEl) {
      this.mountClerkUI();
    } else {
      this.pendingMount = true;
    }
  }

  private mountClerkUI() {
    this.clerkService.mountSignIn(this.clerkSignInEl.nativeElement);
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

