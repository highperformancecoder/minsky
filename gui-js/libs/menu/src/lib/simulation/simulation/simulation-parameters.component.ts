import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { CommunicationService, ElectronService } from '@minsky/core';
import { commandsMapping } from '@minsky/shared';

@Component({
  selector: 'minsky-simulation-parameters',
  templateUrl: './simulation-parameters.component.html',
  styleUrls: ['./simulation-parameters.component.scss'],
})
export class SimulationParametersComponent implements OnInit {
  form: FormGroup;

  constructor(
    private electronService: ElectronService,
    private communicationService: CommunicationService
  ) {
    this.form = new FormGroup({
      timeUnit: new FormControl(null),
      minStepSize: new FormControl(null),
      maxStepSize: new FormControl(null),
      noOfStepsPerIteration: new FormControl(null),
      startTime: new FormControl(null),
      runUntilTime: new FormControl(null),
      absoluteError: new FormControl(null),
      relativeError: new FormControl(null),
      solverOrder: new FormControl(null),
      implicitSolver: new FormControl(null),
    });
  }

  async ngOnInit() {
    await this.makeForm();
  }

  async makeForm() {
    const timeUnit = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.TIME_UNIT,
    });

    const minStepSize = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.STEP_MIN,
    });

    const maxStepSize = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.STEP_MAX,
    });

    const noOfStepsPerIteration = await this.electronService.sendMinskyCommandAndRender(
      { command: commandsMapping.SIMULATION_SPEED }
    );

    const startTime = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.T_ZERO,
    });

    const runUntilTime = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.T_MAX,
    });

    const absoluteError = await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.EPS_ABS,
      }
    );

    const relativeError = await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.EPS_REL,
      }
    );

    const solverOrder = await this.electronService.sendMinskyCommandAndRender({
      command: commandsMapping.ORDER,
    });

    const implicitSolver = await this.electronService.sendMinskyCommandAndRender(
      {
        command: commandsMapping.IMPLICIT,
      }
    );

    this.form.setValue({
      timeUnit,
      minStepSize,
      maxStepSize,
      noOfStepsPerIteration,
      startTime,
      runUntilTime,
      absoluteError,
      relativeError,
      solverOrder,
      implicitSolver,
    });
  }

  async handleSubmit() {
    if (this.electronService.isElectron) {
      const formValues = this.form.value;

      for (const key of Object.keys(formValues)) {
        const arg = formValues[key];

        switch (key) {
          case 'timeUnit':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.TIME_UNIT} "${arg}"`,
            });
            break;

          case 'minStepSize':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.STEP_MIN} ${arg}`,
            });
            break;

          case 'maxStepSize':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.STEP_MAX} ${arg}`,
            });
            break;

          case 'noOfStepsPerIteration':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.SIMULATION_SPEED} ${arg}`,
            });
            break;

          case 'startTime':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.T_ZERO} ${arg}`,
            });
            break;

          case 'runUntilTime':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.T_MAX} ${arg}`,
            });

            break;

          case 'absoluteError':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.EPS_ABS} ${arg}`,
            });
            break;

          case 'relativeError':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.EPS_REL} ${arg}`,
            });
            break;

          case 'solverOrder':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.ORDER} ${arg}`,
            });
            break;

          case 'implicitSolver':
            await this.electronService.sendMinskyCommandAndRender({
              command: `${commandsMapping.IMPLICIT} ${arg}`,
            });
            break;

          default:
            break;
        }
      }
    }

    this.closeWindow();
  }

  closeWindow() {
    if (this.electronService.isElectron) {
      this.electronService.remote.getCurrentWindow().close();
    }
  }
}
