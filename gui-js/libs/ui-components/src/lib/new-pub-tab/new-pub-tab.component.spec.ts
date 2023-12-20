import { ComponentFixture, TestBed } from '@angular/core/testing';

import { NewPubTabComponent } from './new-pub-tab.component';

describe('NewPubTabComponent', () => {
  let component: NewPubTabComponent;
  let fixture: ComponentFixture<NewPubTabComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [NewPubTabComponent],
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
