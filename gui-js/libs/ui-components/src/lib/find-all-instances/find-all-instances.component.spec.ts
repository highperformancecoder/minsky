import { ComponentFixture, TestBed } from '@angular/core/testing';
import { FindAllInstancesComponent } from './find-all-instances.component';

describe('FindAllInstancesComponent', () => {
  let component: FindAllInstancesComponent;
  let fixture: ComponentFixture<FindAllInstancesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [FindAllInstancesComponent],
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(FindAllInstancesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
