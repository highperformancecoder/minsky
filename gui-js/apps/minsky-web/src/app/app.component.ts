import { Component, HostListener, DoCheck, ChangeDetectorRef } from '@angular/core';
import { Router } from '@angular/router';
import { CommunicationService, ElectronService, WindowUtilityService } from '@minsky/core';
import { events, MainRenderingTabs, RenderNativeWindow } from '@minsky/shared';
import { TranslateService } from '@ngx-translate/core';

@Component({
  selector: 'minsky-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss'],
})
export class AppComponent implements DoCheck {
  htmlTabs = [MainRenderingTabs.summary];

  loading = true;
    MainRenderingTabs = MainRenderingTabs;
    private windowUtilityService: WindowUtilityService;
    private resizeTimeout;
  constructor(
    private electronService: ElectronService,
    private cmService: CommunicationService,
    private translate: TranslateService,
    private cdRef: ChangeDetectorRef,
    public router: Router
  ) {
    this.windowUtilityService=new WindowUtilityService(electronService);
    this.translate.setDefaultLang('en');
  }

  @HostListener('window:resize', ['$event'])
  onResize(event) {
    if(!this.htmlTabs.includes(this.cmService.currentTab)) {
      if(!this.resizeTimeout) {
        this.cmService.setWindowSizeAndCanvasOffsets();
        this.resizeTimeout = setTimeout(() => {}, 300);
      } else {
        clearTimeout(this.resizeTimeout);
        this.resizeTimeout = setTimeout(() => {
          this.cmService.setWindowSizeAndCanvasOffsets();
          clearTimeout(this.resizeTimeout);
        }, 300);
      }
    }
  }

  ngDoCheck() {
    if(this.loading && this.router.url !== '/') {
      this.loading = false;
      this.cdRef.detectChanges();

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

  async changeTab(tab: MainRenderingTabs) {
    if(this.htmlTabs.includes(tab)) {
      if(!this.htmlTabs.includes(this.cmService.currentTab)) {
        new RenderNativeWindow(this.cmService.currentTab).$callMethodSync("disable");
      }
  
      this.cmService.currentTab = tab;

      this.router.navigate([tab]);
    } else {
      this.router.navigate(['/wiring']);

      
      if (this.electronService.isElectron) {
        if(!this.htmlTabs.includes(this.cmService.currentTab)) {
          await new RenderNativeWindow(this.cmService.currentTab).requestRedraw();
        }
    
        this.cmService.currentTab = tab;

        setTimeout(async ()=> {
          await this.windowUtilityService.reInitialize();
          var container=this.windowUtilityService.getMinskyContainerElement();
          const scrollableArea=this.windowUtilityService.getScrollableArea();
          container.scrollTop=scrollableArea.height / 2;
          container.scrollLeft=scrollableArea.width / 2;
          const payload = { newTab: tab };
          await this.electronService.send(events.CHANGE_MAIN_TAB, payload);
          this.cmService.resetScroll();
        }, 1);
      }
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
