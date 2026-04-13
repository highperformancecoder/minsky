import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { MatButtonModule } from '@angular/material/button';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
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
    FormsModule,
    MatButtonModule,
    MatFormFieldModule,
    MatInputModule,
    MatProgressSpinnerModule,
  ],
})
export class LoginComponent implements OnInit {
  isLoading = true;
  isSigningIn = false;
  errorMessage = '';
  isAuthenticated = false;
  email = '';
  password = '';

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
    } catch (err) {
      this.errorMessage = 'Session expired. Please sign in again.';
      this.isAuthenticated = false;
    } finally {
      this.isLoading = false;
    }
  }

  async onSignIn() {
    this.isSigningIn = true;
    this.errorMessage = '';
    try {
      await this.clerkService.signInWithEmailPassword(this.email, this.password);
      this.isAuthenticated = true;
      this.electronService.closeWindow();
    } catch (err: any) {
      this.errorMessage = err?.message ?? 'Sign in failed.';
    } finally {
      this.isSigningIn = false;
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

