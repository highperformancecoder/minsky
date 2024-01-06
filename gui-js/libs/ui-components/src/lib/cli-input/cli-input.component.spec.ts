import { ComponentFixture, TestBed } from '@angular/core/testing';

import { CliInputComponent } from './cli-input.component';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('CliInputComponent', () => {
  let component: CliInputComponent;
  let fixture: ComponentFixture<CliInputComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [CliInputComponent, NoopAnimationsModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(CliInputComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
