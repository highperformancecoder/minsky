import { ComponentFixture, TestBed } from '@angular/core/testing';

import { NewPubTabComponent } from './new-pub-tab.component';
import { RouterTestingModule } from '@angular/router/testing';

describe('NewPubTabComponent', () => {
  let component: NewPubTabComponent;
  let fixture: ComponentFixture<NewPubTabComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [NewPubTabComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewPubTabComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
