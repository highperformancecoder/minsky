globalThis.ngJest = {
    testEnvironmentOptions: {
      errorOnUnknownElements: true,
      errorOnUnknownProperties: true,
    },
  };
import 'jest-preset-angular/setup-env/zone';
import '../../../libs/shared/src/test-setup';

import { TestBed } from '@angular/core/testing';
import { BrowserDynamicTestingModule, platformBrowserDynamicTesting } from '@angular/platform-browser-dynamic/testing';

// Initialize the Angular testing environment for Jest.
// Without this, TestBed.configureTestingModule() throws:
// "Need to call TestBed.initTestEnvironment() first"
TestBed.initTestEnvironment(
  BrowserDynamicTestingModule,
  platformBrowserDynamicTesting(),
);
