import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { CommunicationService, ElectronService } from '@minsky/core';
import { MatButtonModule } from '@angular/material/button';
//import { commandsMapping } from '@minsky/shared';

@Component({
    selector: 'minsky-simulation-parameters',
    templateUrl: './simulation-parameters.component.html',
    styleUrls: ['./simulation-parameters.component.scss'],
    standalone: true,
    imports: [
        FormsModule,
        ReactiveFormsModule,
        MatButtonModule,
    ],
})
export class SimulationParametersComponent implements OnInit {
  form: FormGroup;

  constructor(
    private electronService: ElectronService,
    private communicationService: CommunicationService
  ) {
    this.form = new FormGroup({
      timeUnit: new FormControl(''),
      minStepSize: new FormControl(0),
      maxStepSize: new FormControl(0),
      noOfStepsPerIteration: new FormControl(1),
      startTime: new FormControl(0),
      runUntilTime: new FormControl(Infinity),
      absoluteError: new FormControl(0),
      relativeError: new FormControl(0),
      solverOrder: new FormControl(4),
      implicitSolver: new FormControl(false),
    });
  }

  async ngOnInit() {
    await this.makeForm();
  }

  async makeForm() {
    let minsky=this.electronService.minsky;
    const timeUnit = await minsky.timeUnit();
    const minStepSize = await minsky.stepMin();
    const maxStepSize = await minsky.stepMax();
    const noOfStepsPerIteration = await minsky.nSteps();
    const startTime = await minsky.t0();
    const runUntilTime = await minsky.tmax();
    const absoluteError = await minsky.epsAbs();
    const relativeError = await minsky.epsRel();
    const solverOrder = await minsky.order();
    const implicitSolver = await minsky.implicit();

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
      let minsky=this.electronService.minsky;

      minsky.timeUnit(formValues['timeUnit']);
      minsky.stepMin(formValues['minStepSize']);
      minsky.stepMax(formValues['maxStepSize']);
      minsky.nSteps(formValues['noOfStepsPerIteration']);
      minsky.t0(formValues['startTime']);
      // runUntilTime done as a text input, to allow Infinity
      minsky.tmax(Number(formValues['runUntilTime']));
      minsky.epsAbs(formValues['absoluteError']);
      minsky.epsRel(formValues['relativeError']);
      minsky.order(formValues['solverOrder']);
      minsky.implicit(formValues['implicitSolver']);
    }

    this.closeWindow();
  }

  closeWindow() {this.electronService.closeWindow();}
}
