import { ComponentFixture, TestBed } from '@angular/core/testing';

import { VariablePaneComponent } from './variable-pane.component';

describe('VariablePaneComponent', () => {
  let component: VariablePaneComponent;
  let fixture: ComponentFixture<VariablePaneComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [VariablePaneComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(VariablePaneComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
