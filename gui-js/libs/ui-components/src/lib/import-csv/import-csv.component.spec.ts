import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ImportCsvComponent } from './import-csv.component';
import { ElectronService } from '@minsky/core';
import { RouterTestingModule } from '@angular/router/testing';
import { ChangeDetectorRef } from '@angular/core';

describe('ImportCsvComponent', () => {
  let component: ImportCsvComponent;
  let fixture: ComponentFixture<ImportCsvComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      providers: [ElectronService, ChangeDetectorRef],
      imports: [ImportCsvComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(ImportCsvComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
