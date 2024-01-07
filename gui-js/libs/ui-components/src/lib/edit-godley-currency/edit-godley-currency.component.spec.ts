import { ComponentFixture, TestBed } from '@angular/core/testing';

import { EditGodleyCurrencyComponent } from './edit-godley-currency.component';

describe('EditGodleyCurrencyComponent', () => {
  let component: EditGodleyCurrencyComponent;
  let fixture: ComponentFixture<EditGodleyCurrencyComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [EditGodleyCurrencyComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(EditGodleyCurrencyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
