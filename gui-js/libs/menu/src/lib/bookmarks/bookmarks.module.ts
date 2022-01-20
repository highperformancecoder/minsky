import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { SharedModule } from '@minsky/shared';
import { AddBookmarkComponent } from './add-bookmark/add-bookmark.component';
import { BookmarksRoutingModule } from './bookmarks-routing.module';

@NgModule({
  declarations: [AddBookmarkComponent],
  imports: [CommonModule, BookmarksRoutingModule, SharedModule],
})
export class BookmarksModule {}
