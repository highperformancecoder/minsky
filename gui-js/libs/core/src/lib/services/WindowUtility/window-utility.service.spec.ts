import { TestBed } from '@angular/core/testing';

import { WindowUtilityService } from './window-utility.service';

describe('WindowUtilityService', () => {
  let service: WindowUtilityService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WindowUtilityService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
