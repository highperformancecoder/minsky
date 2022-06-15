import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BookmarksModule } from './bookmarks/bookmarks.module';
import { ContextMenuModule } from './context-menu/context-menu.module';
import { EditModule } from './edit/edit.module';
import { FileModule } from './file/file.module';
import { InsertModule } from './insert/insert.module';
import { MenuRoutingModule } from './menu-routing.module';
import { OptionsModule } from './options/options.module';
import { SimulationModule } from './simulation/simulation.module';

@NgModule({
  imports: [
    CommonModule,
    FileModule,
    MenuRoutingModule,
    EditModule,
    InsertModule,
    OptionsModule,
    SimulationModule,
    BookmarksModule,
    ContextMenuModule,
  ],
  declarations: [],
  exports: [],
})
export class MenuModule {}
