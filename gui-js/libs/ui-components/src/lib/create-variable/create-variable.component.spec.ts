import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';

import { CreateVariableComponent } from './create-variable.component';

describe('CreateVariableComponent', () => {
  let component: CreateVariableComponent;
  let fixture: ComponentFixture<CreateVariableComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [CreateVariableComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(CreateVariableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
