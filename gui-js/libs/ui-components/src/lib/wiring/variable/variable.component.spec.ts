import { ComponentFixture, TestBed } from '@angular/core/testing';

import { VariableComponent } from './variable.component';

describe('VariableComponent', () => {
  let component: VariableComponent;
  let fixture: ComponentFixture<VariableComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [VariableComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(VariableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
