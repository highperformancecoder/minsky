import { enableProdMode, importProvidersFrom } from '@angular/core';

import { HttpLoaderFactory } from './app/app.module';
import { AppConfig } from './environments/environment';
import { AppComponent } from './app/app.component';
import { TranslateModule, TranslateLoader } from '@ngx-translate/core';

import { AppRoutingModule } from './app/app-routing.module';
import { MatInputModule } from '@angular/material/input';
import { MatDialogModule } from '@angular/material/dialog';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';

import { withInterceptorsFromDi, provideHttpClient, HttpClient, HttpClientModule } from '@angular/common/http';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { BrowserAnimationsModule, provideAnimations } from '@angular/platform-browser/animations';
import { BrowserModule, bootstrapApplication } from '@angular/platform-browser';

if (AppConfig.production) {
  enableProdMode();
}


bootstrapApplication(AppComponent, {
    providers: [
    provideAnimations(),
    provideHttpClient(withInterceptorsFromDi()),
    importProvidersFrom(TranslateModule.forRoot({
      loader: {
        provide: TranslateLoader,
        useFactory: HttpLoaderFactory,
        deps: [HttpClient],
      },
    }),
    HttpClientModule,
    AppRoutingModule)
]
})
  .catch((err) => console.error(err));
