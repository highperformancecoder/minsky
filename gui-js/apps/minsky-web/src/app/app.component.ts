import { Component, HostListener, DoCheck, ChangeDetectorRef, OnInit } from '@angular/core';
import { Router, RouterOutlet } from '@angular/router';
import { CommunicationService, ElectronService, WindowUtilityService } from '@minsky/core';
import { events, RenderNativeWindow } from '@minsky/shared';
import { TranslateService } from '@ngx-translate/core';
import { MatButtonModule } from '@angular/material/button';
import { HeaderComponent } from '../../../../libs/ui-components/src/lib/header/header.component';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';


@Component({
    selector: 'minsky-root',
    templateUrl: './app.component.html',
    styleUrls: ['./app.component.scss'],
    standalone: true,
    imports: [
    MatProgressSpinnerModule,
    RouterOutlet,
    HeaderComponent,
    MatButtonModule
],
})
export class AppComponent implements OnInit, DoCheck {
  htmlTabs = ['itemTab/summary'];

  loading = true;
  publicationTabs = [];
  private windowUtilityService: WindowUtilityService;
  private resizeTimeout;
  constructor(
    private electronService: ElectronService,
    private cmService: CommunicationService,
    private translate: TranslateService,
    private cdRef: ChangeDetectorRef,
    public router: Router,
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

  async ngDoCheck() {
    if(this.loading && this.router.url !== '/') {
      this.loading = false;
      this.updatePubTabs();
      this.cdRef.detectChanges();

      let handleKey=(event)=>{
        switch (event.key) {
        case 'Escape':
          this.handleEscKey(event);
          break;
      
        case 'Enter':
          event.preventDefault();
          this.handleEnterKey(event);
          break;
      
        default:
          break;
        }
      }
      
      // When the event DOMContentLoaded occurs, it is safe to access the DOM
      document.addEventListener('DOMContentLoaded', async () => {
        await this.cmService.setWindowSizeAndCanvasOffsets();
        // a bit kludgy, but this allows reverting back to default key handling when an HTML element with class defaultEscapeEnter is present. See edit-description component for an example.
        if (document.getElementsByClassName('defaultEscapeEnter').length) {
          document.removeEventListener('keydown', handleKey);
        }
      });

      document.addEventListener('keydown', handleKey);
      this.cmService.setBackgroundColor();

    }  
  }

  async ngOnInit() {
    this.electronService.on(events.CHANGE_MAIN_TAB, ()=>this.changeTab('minsky.canvas'));
    this.electronService.on(events.PUB_TAB_REMOVED, ()=>this.updatePubTabs());
  }

  async updatePubTabs() {
    let pubTabs=await this.electronService.minsky.publicationTabs.$properties();
    this.publicationTabs=[];
    for (let i=0; i<pubTabs.length; ++i)
      this.publicationTabs.push(pubTabs[i].name);
    this.cdRef.detectChanges();
  }
  
  async addPubTab() {
    await this.electronService.invoke(events.NEW_PUB_TAB);
    this.updatePubTabs();
    this.changeTab('minsky.canvas');
  }

  pubTabContext(event,command) {
    this.electronService.send(events.CONTEXT_MENU,
                                    {
                                      x: event.clientX,
                                      y: event.clientY,
                                      type: "publication-button",
                                      command: command,
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
      buttons.forEach((b) => {
          b.click();
      });
  }

  async changeTab(tab: string) {
    if(tab === this.cmService.currentTab) return;

    if(this.htmlTabs.includes(tab)) {
      if(!this.htmlTabs.includes(this.cmService.currentTab)) {
        new RenderNativeWindow(this.cmService.currentTab).$callMethodSync("destroyFrame");
      }
  
      this.cmService.currentTab = tab;

      this.router.navigate([tab]);
    } else {
      this.router.navigate(['wiring']);

      if (this.electronService.isElectron) {
        this.cmService.currentTab = tab;
        
        setTimeout(async ()=> {
          await this.windowUtilityService.reInitialize();
          const payload = { newTab: tab };
          await this.electronService.invoke(events.CHANGE_MAIN_TAB, payload);
          var container=this.windowUtilityService.getMinskyContainerElement();
          const scrollableArea=this.windowUtilityService.getScrollableArea();
          container.scrollTop=scrollableArea.height / 2;
          container.scrollLeft=scrollableArea.width / 2;
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
