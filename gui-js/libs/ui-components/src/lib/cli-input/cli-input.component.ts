import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormControl,
  FormGroup,
  Validators,
} from '@angular/forms';
import { ElectronService } from '@minsky/core';
import { commandsMapping, unExposedTerminalCommands } from '@minsky/shared';
import { AutoUnsubscribe } from 'ngx-auto-unsubscribe';
import { Observable } from 'rxjs';
import { map, startWith } from 'rxjs/operators';

@AutoUnsubscribe()
@Component({
  selector: 'minsky-cli-input',
  templateUrl: './cli-input.component.html',
  styleUrls: ['./cli-input.component.scss'],
})
export class CliInputComponent implements OnInit, OnDestroy {
  commands: Array<string>;
  filteredOptions$: Observable<string[]>;
  command: string;
  form: FormGroup;
  output = [];

  public get commandControl(): AbstractControl {
    return this.form.get('command');
  }

  public get argsControl(): AbstractControl {
    return this.form.get('args');
  }

  constructor(private electronService: ElectronService) {}

  async ngOnInit() {
    this.form = new FormGroup({
      command: new FormControl('', Validators.required),
      args: new FormControl(),
    });

    this.filteredOptions$ = this.commandControl.valueChanges.pipe(
      startWith(''),
      map((value) => this._filter(value))
    );

    this.form.valueChanges.subscribe(() => {
      this.command = this.makeCommand();
    });

    if (this.electronService.isElectron) {
      let _commands = (await this.electronService.sendMinskyCommandAndRender({
        command: commandsMapping.LIST_V2,
      })) as string[];

      _commands = _commands.map((c) => `/minsky${c}`);

      this.commands = [..._commands, ...unExposedTerminalCommands];
    }
  }

  async handleSubmit() {
    if (this.electronService.isElectron && this.command) {
      const output = await this.electronService.sendMinskyCommandAndRender({
        command: this.command,
      });

      this.output.push(`${this.command} ==> ${JSON.stringify(output)}`);
    }
  }

  private makeCommand() {
    return `${this.commandControl.value} ${
      this.argsControl.value || ''
    }`.trim();
  }

  private _filter(value: string): string[] {
    const filterValue = value.toLowerCase();

    return this.commands?.filter((option) =>
      option.toLowerCase().includes(filterValue)
    );
  }

  // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
  ngOnDestroy() {}
}
