import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { WiringComponent } from './wiring.component';

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
});
