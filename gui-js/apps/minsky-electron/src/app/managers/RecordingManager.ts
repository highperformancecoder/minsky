import {
  commandsMapping,
  events,
  normalizeFilePathForPlatform,
  RecordingStatus,
} from '@minsky/shared';
import { ipcMain } from 'electron';
import { dialog, MessageBoxSyncOptions } from 'electron/main';
import { createWriteStream, readFileSync, WriteStream } from 'fs';
import * as JSONStream from 'JSONStream';
import { WindowManager } from './WindowManager';

export class RecordingManager {
  static isRecording = false;
  private static recordingWriteStream: WriteStream;
  private static recordingFilePath: string;
  private static JSONStreamWriter;

  static async handleRecordingReplay() {
    this.stopRecording();

    const replayRecordingDialog = await dialog.showOpenDialog({
      filters: [
        { extensions: ['json'], name: 'JSON' },
        { extensions: ['*'], name: 'All Files' },
      ],
    });

    if (replayRecordingDialog.canceled) {
      return;
    }

    const positiveResponseText = 'Yes';
    const negativeResponseText = 'No';
    const cancelResponseText = 'Cancel';

    const options: MessageBoxSyncOptions = {
      buttons: [positiveResponseText, negativeResponseText, cancelResponseText],
      message: 'Do you want to save the current model?',
      title: 'Save ?',
    };

    const index = dialog.showMessageBoxSync(options);

    if (options.buttons[index] === positiveResponseText) {
      const saveDialog = await dialog.showSaveDialog({});

      const { canceled, filePath: _filePath } = saveDialog;
      const filePath = normalizeFilePathForPlatform(_filePath);

      if (canceled || !filePath) {
        return;
      }

      ipcMain.emit(events.MINSKY_PROCESS_FOR_IPC_MAIN, null, {
        command: `${commandsMapping.SAVE} ${saveDialog.filePath}`,
      });

      await this.replay(replayRecordingDialog);
    }

    if (options.buttons[index] === negativeResponseText) {
      await this.replay(replayRecordingDialog);
    }

    return;
  }

  private static async replay(
    replayRecordingDialog: Electron.OpenDialogReturnValue
  ) {
    const filePath = replayRecordingDialog.filePaths[0];
    const replayFile = readFileSync(filePath, {
      encoding: 'utf8',
      flag: 'r',
    });

    const replayJSON = JSON.parse(replayFile);

    WindowManager.getMainWindow().webContents.send(events.REPLAY_RECORDING, {
      json: replayJSON,
    });
  }

  static record(command: string) {
    const payload = { command: command, executedAt: Date.now() };

    if (!this.recordingWriteStream) {
      this.recordingWriteStream = createWriteStream(this.recordingFilePath);
      this.JSONStreamWriter = JSONStream.stringify();
      this.JSONStreamWriter.pipe(this.recordingWriteStream);
    }

    const recordIgnoreCommands = [
      commandsMapping.CANVAS_GET_ITEM_AT,
      commandsMapping.CANVAS_GET_ITEM_AT_FOCUS,
      commandsMapping.CANVAS_GET_WIRE_AT,
      commandsMapping.START_MINSKY_PROCESS,
      commandsMapping.RECORD,
      commandsMapping.RECORDING_REPLAY,
    ];

    if (!recordIgnoreCommands.find((cmd) => command.includes(cmd)))
      this.JSONStreamWriter.write(payload);
  }

  private static stopRecording() {
    this.isRecording = false;

    if (this.recordingWriteStream) {
      if (this.JSONStreamWriter) {
        this.JSONStreamWriter.end();
        this.JSONStreamWriter = null;
      }
      this.recordingWriteStream.close();
      this.recordingWriteStream = null;
    }

    WindowManager.getMainWindow().webContents.send(
      events.RECORDING_STATUS_CHANGED,
      {
        status: RecordingStatus.RecordingStopped,
      }
    );
  }

  static async handleRecord() {
    if (this.isRecording) {
      this.stopRecording();
      return;
    }

    const saveRecordingDialog = await dialog.showSaveDialog({
      properties: ['showOverwriteConfirmation', 'createDirectory'],
      filters: [
        { extensions: ['json'], name: 'JSON' },
        { extensions: ['*'], name: 'All Files' },
      ],
      defaultPath: 'recording.json',
    });

    const { canceled, filePath } = saveRecordingDialog;

    if (canceled || !filePath) {
      WindowManager.getMainWindow().webContents.send(
        events.RECORDING_STATUS_CHANGED,
        {
          status: RecordingStatus.RecordingCanceled,
        }
      );

      return;
    }

    this.isRecording = true;

    this.recordingFilePath = filePath;
    WindowManager.getMainWindow().webContents.send(
      events.RECORDING_STATUS_CHANGED,
      {
        status: RecordingStatus.RecordingStarted,
      }
    );
  }
}
