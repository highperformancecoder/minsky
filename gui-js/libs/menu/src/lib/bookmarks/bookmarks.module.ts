import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AddBookmarkComponent } from './add-bookmark/add-bookmark.component';
import { BookmarksRoutingModule } from './bookmarks-routing.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
  declarations: [AddBookmarkComponent],
  imports: [CommonModule, BookmarksRoutingModule, FormsModule, ReactiveFormsModule],
})
export class BookmarksModule {}
