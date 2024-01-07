import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { RenameAllInstancesComponent } from './rename-all-instances.component';

describe('RenameAllInstancesComponent', () => {
  let component: RenameAllInstancesComponent;
  let fixture: ComponentFixture<RenameAllInstancesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [RenameAllInstancesComponent, RouterTestingModule],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(RenameAllInstancesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
