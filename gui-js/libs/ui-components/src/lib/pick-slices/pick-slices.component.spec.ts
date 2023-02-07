import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PickSlicesComponent } from './pick-slices.component';

describe('PickSlicesComponent', () => {
  let component: PickSlicesComponent;
  let fixture: ComponentFixture<PickSlicesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [PickSlicesComponent],
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
