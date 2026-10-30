import { ComponentFixture, TestBed } from '@angular/core/testing';
import { ActivatedRoute } from '@angular/router';
import { RouterTestingModule } from '@angular/router/testing';
import { ElectronService } from '@minsky/core';
import { minsky } from '@minsky/shared';
import { of } from 'rxjs';
import { VariablePaneComponent } from './variable-pane.component';

// the real canvas pulls in d3 (ESM only), and its rendering is not under test
jest.mock('../svg-canvas/svg-canvas.component', () => {
  const { Component, EventEmitter, Input, Output } = require('@angular/core');
  class SvgCanvasComponent {
    unparkedData: any;
    enableScroll: boolean;
    showConnectors: boolean;
    showValuelines: boolean;
    zoom: number;
    dragStarted = new EventEmitter();
    setDimensions = jest.fn();
  }
  Component({ selector: 'svg-canvas', template: '', standalone: true })(SvgCanvasComponent);
  for (const p of ['unparkedData', 'enableScroll', 'showConnectors', 'showValuelines', 'zoom'])
    Input()(SvgCanvasComponent.prototype, p);
  Output()(SvgCanvasComponent.prototype, 'dragStarted');
  return { SvgCanvasComponent };
});
jest.mock('@minsky/core', () => ({ ElectronService: class {} }));
jest.mock('@minsky/shared', () => ({
  minsky: {
    variableValues: undefined,
    canvas: { addVariable: jest.fn() },
  },
  VariableValue: class {},
  VariableValues: class {},
}));

interface FakeVar {
  name: string;
  type: string;
  value: number;
}

function fakeVariableValues(vars: FakeVar[]) {
  const byKey = Object.fromEntries(vars.map((v) => [v.name, v]));
  return {
    keys: async () => vars.map((v) => v.name),
    elem: (key: string) => {
      const v = byKey[key];
      return {
        type: async () => v.type,
        name: async () => v.name,
        detailedText: async () => `desc of ${v.name}`,
        value: async () => v.value,
        sliderMin: async () => 0,
        sliderMax: async () => 10,
      };
    },
  };
}

// jsdom doesn't implement named property access on forms (form['some-id']),
// which the component relies on, so emulate it via form.elements.
const realFormsDescriptor = Object.getOwnPropertyDescriptor(Document.prototype, 'forms');
function emulateFormNamedAccess() {
  Object.defineProperty(document, 'forms', {
    configurable: true,
    get() {
      const forms = realFormsDescriptor.get.call(document) as HTMLCollectionOf<HTMLFormElement>;
      return new Proxy(forms, {
        get(target, prop, receiver) {
          if (typeof prop === 'string' && !/^\d+$/.test(prop) && !(prop in HTMLCollection.prototype)) {
            const form = target.namedItem(prop);
            if (!form) return undefined;
            return new Proxy(form, {
              get(f, p) {
                if (typeof p === 'string' && !(p in HTMLFormElement.prototype) && f.elements.namedItem(p)) return f.elements.namedItem(p);
                const v = Reflect.get(f, p, f);
                return typeof v === 'function' ? v.bind(f) : v;
              },
            });
          }
          const v = Reflect.get(target, prop, target);
          return typeof v === 'function' ? v.bind(target) : v;
        },
      });
    },
  });
}

describe('VariablePaneComponent', () => {
  let component: VariablePaneComponent;
  let fixture: ComponentFixture<VariablePaneComponent>;
  const mockMinsky = minsky as any;

  async function create(vars: FakeVar[] = []) {
    mockMinsky.variableValues = fakeVariableValues(vars);
    fixture = TestBed.createComponent(VariablePaneComponent);
    component = fixture.componentInstance;
    // the component looks its form up via document.forms, so must be attached
    document.body.appendChild(fixture.nativeElement);
    fixture.detectChanges(); // triggers ngAfterViewInit -> initEvents
    await fixture.whenStable();
  }

  const checkbox = (type: string) =>
    document.forms['variablePane']['variablePane::' + type] as HTMLInputElement;

  beforeAll(emulateFormNamedAccess);
  afterAll(() => delete (document as any).forms);
  afterEach(() => fixture?.nativeElement.remove());

  beforeEach(async () => {
    jest.clearAllMocks();
    await TestBed.configureTestingModule({
      imports: [VariablePaneComponent, RouterTestingModule],
      providers: [
        { provide: ElectronService, useValue: {} },
        {
          provide: ActivatedRoute,
          useValue: { queryParams: of({ itemId: 7, systemWindowId: 42 }) },
        },
      ],
    }).compileComponents();
  });

  it('should create and pick up route query params', async () => {
    await create();
    expect(component).toBeTruthy();
    expect(component.itemId).toBe(7);
    expect(component.systemWindowId).toBe(42 as any);
  });

  describe('findExponent', () => {
    beforeEach(async () => create());

    it.each([
      [0, 0],
      [1, 0],
      [999, 0],
      [1000, 3],
      [123456, 3],
      [1e6, 6],
      [0.5, -3],
      [0.001, -3],
      [-2500, 3],
    ])('findExponent(%p) = %p', (n, e) => {
      expect(component.findExponent(n)).toBe(e);
    });
  });

  describe('initEvents', () => {
    it('checks all type checkboxes', async () => {
      await create();
      for (const t of ['flow', 'parameter', 'stock', 'integral'])
        expect(checkbox(t).checked).toBe(true);
    });

    it('loads supported variables, skipping constants and unsupported types', async () => {
      await create([
        { name: 'a', type: 'flow', value: 5 },
        { name: 'b', type: 'constant', value: 1 },
        { name: 'c', type: 'parameter', value: 2 },
      ]);
      const names = component.allSvgData.map((d) => d.symbol);
      expect(names.sort()).toEqual(['a', 'c']);
    });

    it('strips scope prefix from names', async () => {
      await create([{ name: 'scope:x', type: 'stock', value: 1 }]);
      expect(component.allSvgData[0].symbol).toBe('x');
      expect(component.allSvgData[0].symbol).toBe('x');
    });

    it('normalises value by its exponent', async () => {
      await create([{ name: 'big', type: 'flow', value: 12000 }]);
      const d = component.allSvgData[0];
      expect(d.exponent).toBe(3);
      expect(d.value).toBeCloseTo(12);
      expect(d.minimum).toBe(0);
      expect(d.maximum).toBe(10);
      expect(d.rotation).toBe(0);
    });
  });

  describe('setPositions', () => {
    const vars: FakeVar[] = [
      { name: 'zeta', type: 'flow', value: 1 },
      { name: 'alpha', type: 'flow', value: 1 },
      { name: 'p', type: 'parameter', value: 1 },
      { name: 's', type: 'stock', value: 1 },
    ];

    async function latest() {
      let out: any[];
      component.svgData$.subscribe((v) => (out = v));
      return out;
    }

    it('does nothing before data is loaded', async () => {
      await create();
      component.allSvgData = undefined;
      const before = component.svgData$.value;
      component.setPositions();
      expect(component.svgData$.value).toBe(before);
    });

    it('sorts by type then symbol', async () => {
      await create(vars);
      const out = await latest();
      expect(out.map((d) => d.symbol)).toEqual(['alpha', 'zeta', 'p', 's']);
    });

    it('filters out unchecked types on select()', async () => {
      await create(vars);
      checkbox('flow').checked = false;
      component.select('flow');
      const out = await latest();
      expect(out.map((d) => d.type.name).includes('flow')).toBe(false);
      expect(out.length).toBe(2);
    });

    it('lays out items vertically in 60px steps, starting at y=40', async () => {
      await create(vars);
      const out = await latest();
      // jsdom clientHeight is 0, so column length is 1: one item per column
      out.forEach((d) => expect(d.y).toBe(40));
      const xs = out.map((d) => d.x);
      expect([...xs].sort((a, b) => a - b)).toEqual(xs);
    });

    it('publishes container width', async () => {
      await create(vars);
      expect(component.containerWidth$.value).toMatch(/^\d+(\.\d+)?px$/);
    });
  });

  describe('onDragStarted', () => {
    it('adds the dragged variable to the canvas', async () => {
      await create();
      await component.onDragStarted({ data: { name: 'foo', type: { name: 'flow' } } } as any);
      expect(mockMinsky.canvas.addVariable).toHaveBeenCalledWith('foo', 'flow');
    });
  });

  describe('events and teardown', () => {
    it('converts wheel deltaY to horizontal scroll', async () => {
      await create();
      const el = component.variablePaneContainer;
      el.scrollLeft = 0;
      const evt = new WheelEvent('wheel', { deltaY: 25, cancelable: true });
      el.dispatchEvent(evt);
      expect(evt.defaultPrevented).toBe(true);
      expect(el.scrollLeft).toBe(25);
    });

    it('recomputes positions on window resize', async () => {
      await create();
      const spy = jest.spyOn(component, 'setPositions');
      component.onResize(new Event('resize'));
      expect(spy).toHaveBeenCalled();
    });

    it('completes destroy$ on destroy', async () => {
      await create();
      let done = false;
      component.destroy$.subscribe({ complete: () => (done = true) });
      fixture.destroy();
      expect(done).toBe(true);
    });
  });
});
