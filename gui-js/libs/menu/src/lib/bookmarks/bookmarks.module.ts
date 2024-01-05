import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AddBookmarkComponent } from './add-bookmark/add-bookmark.component';
import { BookmarksRoutingModule } from './bookmarks-routing.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

@NgModule({
    imports: [CommonModule, BookmarksRoutingModule, FormsModule, ReactiveFormsModule, AddBookmarkComponent],
})
export class BookmarksModule {}
