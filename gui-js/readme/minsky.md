# Architecture

This repo is created using [Nx](https://nx.dev). Nx helps us creating a monorepo for all things javascript.

We are also using [nx-electron](https://github.com/bennymeg/nx-electron) to generate/build/serve the electron app.

The project uses Electron with Angular to create the Minsky desktop app for all platforms. There are separate scripts to build and generate the Minsky executable/installer for each platform. The scripts can be found in the package.json file under scripts.

```json
{
  "export:package:windows": "npm run build:web && npm run build:electron && nx run minsky-electron:make --platform windows",
  "export:package:linux": "npm run build:web && npm run build:electron && nx run minsky-electron:make --platform linux --arch x64 --publishPolicy never",
  "export:package:mac": "npm run build:web && npm run build:electron && nx run minsky-electron:make --platform mac"
}
```

---

# Communication between Electron and Angular

We are using electron's IPC to communicate between frontend (angular) and backend (electron). The IPC consists of two main parts

- ipcRenderer (used with the frontend, in our case its angular). [more info](https://www.electronjs.org/docs/latest/api/ipc-renderer)
- ipcMain (used with the backend, i.e electron). [more info ](https://www.electronjs.org/docs/latest/api/ipc-main)

The electron.service.ts in angular consists of all electron related helpers.

```javascript
export class ElectronService {
  ipcRenderer: typeof ipcRenderer;
  remote: typeof remote;
  isElectron = isElectron();
  /* .............. */
}
```

The events sent/emitted using the ipcRenderer can be captured with ipcMain. We are listening for all the events in [electron.events.ts](apps\minsky-electron\src\app\events\electron.events.ts)

---

## Examples of IPC using electron

1. from ipcRenderer to ipcMain - one way - (angular to electron)

```javascript
// angular
this.electronService.ipcRenderer.send(events.SET_BACKGROUND_COLOR, {
  color: color,
});

// electron
ipcMain.on(events.SET_BACKGROUND_COLOR, async (event, { color }) => {
  if (color) {
    StoreManager.store.set('backgroundColor', color);
  }
  await CommandsManager.changeWindowBackgroundColor(
    StoreManager.store.get('backgroundColor')
  );
});
```

1. from ipcRenderer to ipcMain & ipcMain to ipcRenderer - two way - (angular to electron & electron to angular)

```javascript
// angular
const res = await this.ipcRenderer.invoke(events.MINSKY_PROCESS, payload);

// electron
ipcMain.handle(
  events.MINSKY_PROCESS,
  async (event, payload: MinskyProcessPayload) => {
    return await RestServiceManager.handleMinskyProcess(payload);
  }
);
```

1. from ipcMain to ipcMain - one way - (electron to electron)
   we are using this to avoid circular dependency issues.

```javascript
// electron
ipcMain.emit(events.ADD_RECENT_FILE, null, payload.filePath);

// electron
ipcMain.on(events.ADD_RECENT_FILE, (event, filePath: string) => {
  RecentFilesManager.addFileToRecentFiles(filePath);
});
```

---

# Menu

- We use the default way of integrating menus as shown here in the [official documentation](https://www.electronjs.org/docs/latest/api/menu).
- Nothing fancy here.

---

# Context Menu

- context menus are same as menus. we just have to give the x and y of the current mouse position.
- Here we listen for the 'context-menu' event in angular and when that event is triggered we send it to electron.
- check [ContextMenuManager.ts](..\apps\minsky-electron\src\app\managers\ContextMenuManager.ts) for context menu related code.

---

# Folder structure

## [folder structure](FOLDER_STRUCTURE.md)

# Other important packages that we are using

- Nx - for monorepo - https://nx.dev/
- nx-electron - generate/build/serve electron app - https://github.com/bennymeg/nx-electron
- electron-store - local storage for electron - https://www.npmjs.com/package/electron-store
- electron-log - to show/store logs - https://www.npmjs.com/package/electron-log
- compodoc - for documentation generation using code - https://compodoc.app/
