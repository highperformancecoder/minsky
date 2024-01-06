import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PickSlicesComponent } from './pick-slices.component';
import { RouterTestingModule } from '@angular/router/testing';
import { ElectronService } from '@minsky/core';

describe('PickSlicesComponent', () => {
  let component: PickSlicesComponent;
  let fixture: ComponentFixture<PickSlicesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [PickSlicesComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PickSlicesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
