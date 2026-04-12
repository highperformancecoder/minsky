import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormControl, FormGroup, Validators, ReactiveFormsModule } from '@angular/forms';
import { MatButtonModule } from '@angular/material/button';
import { MatCheckboxModule } from '@angular/material/checkbox';
import { MatInputModule } from '@angular/material/input';
import { MatFormFieldModule } from '@angular/material/form-field';
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
    ReactiveFormsModule,
    MatButtonModule,
    MatCheckboxModule,
    MatInputModule,
    MatFormFieldModule,
    MatProgressSpinnerModule,
  ],
})
export class LoginComponent implements OnInit {
  loginForm = new FormGroup({
    email: new FormControl('', [Validators.required, Validators.email]),
    password: new FormControl('', [Validators.required]),
  });

  isLoading = false;
  errorMessage = '';
  isAuthenticated = false;
  showPassword = false;
  isOAuthLoading: 'github' | 'google' | null = null;

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

  private async initializeSession(authToken: string | undefined) {
    try {
      await this.clerkService.initialize();

      if (authToken) {
        await this.clerkService.setSession(authToken);
      }

      this.isAuthenticated = await this.clerkService.isSignedIn();
    } catch (err) {
      this.errorMessage = 'Session expired. Please sign in again.';
      this.isAuthenticated = false;
    }
  }

  get email() {
    return this.loginForm.get('email');
  }

  get password() {
    return this.loginForm.get('password');
  }

  togglePasswordVisibility() {
    this.showPassword = !this.showPassword;
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

  async onSignInWithGitHub() {
    this.isOAuthLoading = 'github';
    this.errorMessage = '';
    try {
      await this.clerkService.signInWithOAuth('oauth_github');
    } catch (err: any) {
      this.errorMessage = err?.errors?.[0]?.message ?? err?.message ?? 'GitHub sign-in failed.';
    } finally {
      this.isOAuthLoading = null;
    }
  }

  async onSignInWithGoogle() {
    this.isOAuthLoading = 'google';
    this.errorMessage = '';
    try {
      await this.clerkService.signInWithOAuth('oauth_google');
    } catch (err: any) {
      this.errorMessage = err?.errors?.[0]?.message ?? err?.message ?? 'Google sign-in failed.';
    } finally {
      this.isOAuthLoading = null;
    }
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
