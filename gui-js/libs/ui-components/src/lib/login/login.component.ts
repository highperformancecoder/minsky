import { ChangeDetectorRef,Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormControl, FormGroup, Validators, ReactiveFormsModule } from '@angular/forms';
import { MatButtonModule } from '@angular/material/button';
import { MatInputModule } from '@angular/material/input';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';
import { MatDividerModule } from '@angular/material/divider';
import { MatTooltipModule } from '@angular/material/tooltip';
import { ClerkService } from '@minsky/core';
import { ElectronService } from '@minsky/core';
import { events } from '@minsky/shared';
import { ActivatedRoute } from '@angular/router';
import { take } from 'rxjs';

@Component({
  selector: 'minsky-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss'],
  standalone: true,
  imports: [
    CommonModule,
    ReactiveFormsModule,
    MatButtonModule,
    MatInputModule,
    MatFormFieldModule,
    MatProgressSpinnerModule,
    MatDividerModule,
    MatTooltipModule,
  ],
})
export class LoginComponent implements OnInit, OnDestroy {
  loginForm = new FormGroup({
    email: new FormControl('', [Validators.required, Validators.email]),
    password: new FormControl('', [Validators.required]),
  });

  isLoading = false;
  errorMessage = '';
  isAuthenticated = false;
  oauthStrategies: string[] = [];

  private readonly PROVIDER_LABELS: Record<string, string> = {
    oauth_github: 'GitHub',
    oauth_google: 'Google',
    oauth_apple: 'Apple',
    oauth_microsoft: 'Microsoft',
    oauth_facebook: 'Facebook',
    oauth_twitter: 'X / Twitter',
    oauth_linkedin: 'LinkedIn',
    oauth_discord: 'Discord',
  };

  private oauthCallbackListener: ((_event: any, callbackUrl: string) => void) | null = null;

  constructor(
    private clerkService: ClerkService,
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private changeDetectorRef: ChangeDetectorRef
  ) {}

  async ngOnInit() {
    this.route.queryParams.pipe(take(1)).subscribe((params) => {
      this.initializeSession(params['authToken']);
    });

    if (this.electronService.isElectron) {
      this.oauthCallbackListener = async (_event: any, callbackUrl: string) => {
        this.isLoading = true;
        try {
          await this.clerkService.handleOAuthCallback(callbackUrl);
          this.isAuthenticated = true;
          this.electronService.closeWindow();
        } catch (err: any) {
          this.errorMessage = err?.message ?? 'OAuth sign-in failed.';
        } finally {
          this.isLoading = false;
        }
      };
      this.electronService.on(events.OAUTH_CALLBACK, this.oauthCallbackListener);
    }
  }

  ngOnDestroy() {
    if (this.oauthCallbackListener) {
      this.electronService.removeListener(events.OAUTH_CALLBACK, this.oauthCallbackListener);
      this.oauthCallbackListener = null;
    }
  }

  private async initializeSession(authToken: string | undefined) {
    try {
      await this.clerkService.initialize();

      if (authToken) {
        await this.clerkService.setSession(authToken);
      }

      this.isAuthenticated = await this.clerkService.isSignedIn();
      this.oauthStrategies = this.clerkService.getSupportedOAuthStrategies();
    } catch (err) {
      this.errorMessage = 'Session expired. Please sign in again.';
      this.isAuthenticated = false;
    }
    this.changeDetectorRef.detectChanges();
  }

  getProviderLabel(strategy: string): string {
    return this.PROVIDER_LABELS[strategy]
      ?? strategy.replace(/^oauth_/, '').replace(/_/g, ' ');
  }

  get email() {
    return this.loginForm.get('email');
  }

  get password() {
    return this.loginForm.get('password');
  }

  async onSubmit() {
    if (this.loginForm.invalid) return;

    this.isLoading = true;
    this.errorMessage = '';

    try {
      await this.clerkService.signInWithEmailPassword(
        this.loginForm.value.email,
        this.loginForm.value.password
      );
      this.isAuthenticated = true;
      this.electronService.closeWindow();
    } catch (err: any) {
      this.errorMessage = err?.errors?.[0]?.message ?? err?.message ?? 'Authentication failed.';
    } finally {
      this.isLoading = false;
    }
  }

  async onOAuthSignIn(strategy: string) {
    this.isLoading = true;
    this.errorMessage = '';
    try {
      const oauthUrl = await this.clerkService.getOAuthRedirectUrl(strategy);
      await this.electronService.invoke(events.OAUTH_OPEN_POPUP, oauthUrl);
    } catch (err: any) {
      this.errorMessage = err?.errors?.[0]?.long_message ?? err?.errors?.[0]?.message ?? err?.message ?? 'OAuth sign-in failed.';
      this.isLoading = false;
    }
    // isLoading intentionally left true while popup is open; cleared in callback handler
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
