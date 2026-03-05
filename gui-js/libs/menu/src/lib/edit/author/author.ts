import { Component, OnDestroy} from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { Subject, takeUntil } from 'rxjs';
import { InputModalComponent} from '@minsky/ui-components';


@Component({
    selector: 'author',
    templateUrl: './author.html',
    styleUrls: ['./author.scss'],
    standalone: true,
    imports: [InputModalComponent],
})
export class AuthorComponent implements OnDestroy {
  author='';

  destroy$ = new Subject<void>();
  
  constructor(
    private route: ActivatedRoute,
    private electronService: ElectronService,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.author=params.author ?? '';
    });
  }

  async handleEditAuthor(author: string) {
    if (this.electronService.isElectron) {
      await this.electronService.minsky.author(author);
    }
    this.windowUtilityService.closeCurrentWindowIfNotMain();
  }

  ngOnDestroy() {
    this.destroy$.next();
    this.destroy$.complete();
  }
}
