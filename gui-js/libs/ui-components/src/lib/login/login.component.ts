import { Component, OnInit } from '@angular/core';
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
export class LoginComponent implements OnInit {
  isLoading = true;
  errorMessage = '';
  isAuthenticated = false;

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

