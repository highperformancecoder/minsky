import { enableProdMode, importProvidersFrom } from '@angular/core';

import { HttpLoaderFactory } from './app/app.module';
import { AppConfig } from './environments/environment';
import { AppComponent } from './app/app.component';
import { TranslateModule, TranslateLoader } from '@ngx-translate/core';

import { AppRoutingModule } from './app/app-routing.module';

import { withInterceptorsFromDi, provideHttpClient, HttpClient, HttpClientModule, withFetch } from '@angular/common/http';
import { provideAnimations } from '@angular/platform-browser/animations';
import { bootstrapApplication } from '@angular/platform-browser';

if (AppConfig.production) {
  enableProdMode();
}


bootstrapApplication(AppComponent, {
    providers: [
    provideAnimations(),
    provideHttpClient(withInterceptorsFromDi(), withFetch()),
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
