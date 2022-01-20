import { MainRenderingTabs } from '../constants/MainRenderingTabs';
export interface InitializePopupWindowPayload {
  customTitle? : string,
  itemInfo: CanvasItem;
  url: string;
  height?: number;
  width?: number;
  modal?: boolean;
}
export interface MinskyProcessPayload {
  mouseX?: number;
  mouseY?: number;
  filePath?: string;
  command?: string;
  windowId?: number;
  showServiceStartedDialog?: boolean;
  key?: string;
  shift?: boolean;
  capsLock?: boolean;
  ctrl?: boolean;
  alt?: boolean;
  args?: Record<string, unknown>;
  render?: boolean;
  location?: number;
}

export interface ElectronCanvasOffset {
  left: number;
  top: number;
  electronMenuBarHeight: number;
}

export interface AppLayoutPayload {
  isResizeEvent: boolean;
  drawableArea: {
    width: number;
    height: number;
  };
  offset: ElectronCanvasOffset;
}

export interface ChangeTabPayload {
  newTab: MainRenderingTabs;
}

export interface HeaderEvent {
  action: string;
  target: string;
  value?: unknown;
}

export enum ClassType {
  Variable = 'Variable',
  VarConstant = 'VarConstant',
  Operation = 'Operation',
  IntOp = 'IntOp',
  DataOp = 'DataOp',
  PlotWidget = 'PlotWidget',
  GodleyIcon = 'GodleyIcon',
  Group = 'Group',
  Item = 'Item',
  SwitchIcon = 'SwitchIcon',
  Ravel = 'Ravel',
  Lock = 'Lock',
  Sheet = 'Sheet',
  UserFunction = 'UserFunction',
}

export enum RecordingStatus {
  RecordingCanceled = 'RecordingCanceled',
  RecordingStarted = 'RecordingStarted',
  RecordingStopped = 'RecordingStopped',
}

export enum GodleyTableOutputStyles {
  DRCR = 'DRCR',
  SIGN = 'sign',
}

export enum ReplayRecordingStatus {
  ReplayStarted = 'ReplayStarted',
  ReplayStopped = 'ReplayStopped',
  ReplayPaused = 'ReplayPaused',
}

export interface CanvasItem {
  classType: ClassType;
  value: number;
  id: number;
}

export interface CreateWindowPayload extends Electron.BrowserWindowConstructorOptions {
  uid? : number,
  url? : string
}

// export interface CreateWindowPayload {
//   uid?: number;
//   width?: number;
//   height?: number;
//   title: string;
//   modal?: boolean;
//   backgroundColor?: string;
//   url?: string;
// }


export interface TypeValueName {
  type : string,
  value? : string | number,
  name? : string
}