import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PenStylesComponent } from './pen-styles.component';

describe('PenStylesComponent', () => {
  let component: PenStylesComponent;
  let fixture: ComponentFixture<PenStylesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [PenStylesComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PenStylesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
