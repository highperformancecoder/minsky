import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PreferencesComponent } from './preferences.component';

describe('PreferencesComponent', () => {
  let component: PreferencesComponent;
  let fixture: ComponentFixture<PreferencesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [PreferencesComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PreferencesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('updates the Ravel error preference', async () => {
    const electronService = (component as any).electronService;
    electronService.isElectron = true;
    const invoke = jest.spyOn(electronService, 'invoke').mockResolvedValue(undefined);
    component.form.controls.silenceRavelErrors.setValue(true);
    await component.updatePreferences();
    expect(invoke).toHaveBeenCalledWith(
      'update-preferences',
      expect.objectContaining({silenceRavelErrors: true})
    );
  });
});
