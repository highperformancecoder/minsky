import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditOperationComponent } from './edit-operation.component';
import { ElectronService } from '@minsky/core';
import { RouterTestingModule } from '@angular/router/testing';

describe('EditOperationComponent', () => {
  let component: EditOperationComponent;
  let fixture: ComponentFixture<EditOperationComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      providers: [ElectronService],
      imports: [EditOperationComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditOperationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
