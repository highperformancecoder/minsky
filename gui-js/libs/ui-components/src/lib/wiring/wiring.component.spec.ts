import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { WiringComponent } from './wiring.component';
import { events } from '@minsky/shared';

describe('WiringComponent', () => {
  let component: WiringComponent;
  let fixture: ComponentFixture<WiringComponent>;

  beforeEach(
    waitForAsync(() => {
      TestBed.configureTestingModule({
        imports: [WiringComponent],
      }).compileComponents();
    })
  );

  beforeEach(() => {
    fixture = TestBed.createComponent(WiringComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('opens the website instead of adding a Ravel when the library is missing', async () => {
    jest.spyOn(component.electronService.minsky, 'ravelLibraryMissing').mockResolvedValue(true);
    const send = jest.spyOn(component.electronService, 'send').mockImplementation(() => {});
    const add = jest.spyOn(component.electronService.minsky.canvas, 'addRavel').mockResolvedValue();
    await component.addRavel();
    expect(send).toHaveBeenCalledWith(events.OPEN_RAVEL_WEBSITE);
    expect(add).not.toHaveBeenCalled();
  });

  it('attempts creation when the library is not missing', async () => {
    jest.spyOn(component.electronService.minsky, 'ravelLibraryMissing').mockResolvedValue(false);
    const add = jest.spyOn(component.electronService.minsky.canvas, 'addRavel').mockRejectedValue(new Error('Ravel error'));
    await expect(component.addRavel()).rejects.toThrow('Ravel error');
    expect(add).toHaveBeenCalled();
  });
});
