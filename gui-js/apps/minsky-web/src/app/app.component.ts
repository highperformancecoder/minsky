import { AfterViewInit, Component } from '@angular/core';
import { Router } from '@angular/router';
import { CommunicationService, ElectronService, WindowUtilityService } from '@minsky/core';
import { events, MainRenderingTabs, RenderNativeWindow } from '@minsky/shared';
import { TranslateService } from '@ngx-translate/core';

@Component({
  selector: 'minsky-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss'],
})
export class AppComponent implements AfterViewInit {
  loader = false;
    MainRenderingTabs = MainRenderingTabs;
    private windowUtilityService: WindowUtilityService;
  constructor(
    private electronService: ElectronService,
    private cmService: CommunicationService,
    private translate: TranslateService,
    public router: Router
  ) {
    this.windowUtilityService=new WindowUtilityService(electronService);
    this.translate.setDefaultLang('en');
  }

  ngAfterViewInit() {
    // When the event DOMContentLoaded occurs, it is safe to access the DOM
    document.addEventListener('DOMContentLoaded', async () => {
      await this.cmService.setWindowSizeAndCanvasOffsets();
    });

    this.cmService.setBackgroundColor();

    document.addEventListener('keydown', (event) => {
      switch (event.key) {
        case 'Escape':
          this.handleEscKey(event);
          break;

        case 'Enter':
          this.handleEnterKey(event);
          break;

        default:
          break;
      }
    });
  }

  // close modals with ESC
  private async handleEscKey(event: KeyboardEvent) {
    (document.activeElement as HTMLElement).blur();
    //CAVEAT: The blur is needed to prevent main window close (If we try to close a child window when one of its inputs has focus - the main window closes and there is a crash)

    const currentWindow = await this.electronService.getCurrentWindow();
    // disable closing by means of dontCloseOnEscape property
    if (currentWindow.id !== 1 && !currentWindow.dontCloseOnEscape) {
      this.electronService.closeWindow();
      event.preventDefault();
    }
  }

  // submits form with class="submit" when pressed Enter key
  private async handleEnterKey(event: KeyboardEvent) {
      // disable invoking OK button if marked dontCloseOnReturn
    const currentWindow = await this.electronService.getCurrentWindow();
    if (currentWindow.dontCloseOnReturn) return;

      (document.activeElement as HTMLElement).blur();
      //CAVEAT: The blur is needed to prevent main window close (If we try to close a child window when one of its inputs has focus - the main window closes and there is a crash)

      // TODO:: Are there scenarios where we need to pass Enter key to the backend?
      
      const buttons = Array.from(
          document.getElementsByClassName('submit')
      ) as HTMLElement[];
      if (buttons.length > 0) {
          event.preventDefault();
      }
      buttons.forEach((b) => {
          b.click();
      });
  }

  async windowResize() {
    await this.cmService.setWindowSizeAndCanvasOffsets();
  }

  async changeTab(tab: MainRenderingTabs) {
    new RenderNativeWindow(this.cmService.currentTab).requestRedraw();

    this.cmService.currentTab = tab;
    if (this.electronService.isElectron) {
      await this.windowUtilityService.reInitialize();
      var container=this.windowUtilityService.getMinskyContainerElement();
      const scrollableArea=this.windowUtilityService.getScrollableArea();
      container.scrollTop=scrollableArea.height / 2;
      container.scrollLeft=scrollableArea.width / 2;
      const payload = { newTab: tab };
      await this.electronService.send(events.CHANGE_MAIN_TAB, payload);
      this.cmService.resetScroll();
    }
  }

  startTerminal() {
    if (this.electronService.isElectron) {
      this.electronService.send(events.CREATE_MENU_POPUP, {
        title: 'Terminal',
        url: `#/headless/terminal`,
        width: 800,
        height: 668,
        minWidth: 350,
        minHeight: 400,
        modal: false,
      });
    }
  }
}
