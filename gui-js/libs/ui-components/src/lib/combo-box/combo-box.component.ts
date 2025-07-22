import {
  Component, ElementRef, Input, ViewContainerRef, SimpleChanges,
  ViewChild, ContentChild, TemplateRef, AfterViewInit, OnChanges, OnDestroy
} from '@angular/core';
import { NgModel } from '@angular/forms';

@Component({
  selector: 'combo-box',
  template: `
    <!-- made up input to handle input value and tet separately -->
    <input class="editor"
      (blur)="onBlur($event)"
      (focus)="onFocus($event)"
      (input)="onInput($event)"
      (keydown)="onKeydown($event)" />
    <button (click)="openPopup('')" aria-label="Show Options">&#9660;</button>
    <ng-content></ng-content>`,
  styleUrls: [ './combo-box.component.scss' ]
})
export class ComboBoxComponent implements AfterViewInit, OnChanges, OnDestroy {
  @Input() clearInput: boolean;
  @Input() disabled: boolean;
  @ContentChild(NgModel) ngModel: NgModel;
  id = `combobox-${parseInt('' + Math.random()*10000)}`;
  docClickHandler = this.onDocumentClick.bind(this);

  inputOrgEl: HTMLInputElement; // Final input value, it becomes not editable
  inputEl: HTMLInputElement; // display and keyboard input representing inputEl
  ulEl: HTMLUListElement;    // list of options given from user.

  options: Array<{text: string, value: string}> = [];  // made from <ul> and <li>
  savedSelection: any;       // the last selected option

  constructor(private elRef: ElementRef) {}

  ngOnChanges(changes: SimpleChanges) {
    if (!changes.disabled.firstChange) {
      this.inputEl.disabled = changes.disabled.currentValue;
    }
  }

  ngAfterViewInit() {
    if (!this.setElements()) return false; // check if it has <input> and <ul>
    document.addEventListener('click', this.docClickHandler);
    this.options = Array.from(this.ulEl.children).map( (el:any) => { // build options
      let [text, value] = [el.innerText, el.getAttribute('value')];
      (value === undefined) && (value = text);
      return {text: text.trim(), value: value.trim()};
    });
    this.disabled && (this.inputEl.disabled = true);
  }

  ngOnDestroy() {
    document.removeEventListener('click', this.docClickHandler);
  }

  onInput(event) { // when key input, open popup and select an option if matching
    const text = this.inputEl.value;
    this.openPopup(text);
    this.select(event);
  }

  onDocumentClick(event) {
    const combobox = event.target.closest('combo-box');
    const clickInMe = combobox === this.elRef.nativeElement;
    !clickInMe && this.closePopup();
  }

  onBlur(event) { // when leave input field with invalid value, recover the last value
    const text = this.inputEl.value;
    const option = this.options.find(el => el.text === text);
    !option && this.savedSelection && this.savedSelection.value &&
      (this.inputEl.value = this.savedSelection.text);
  }

  onKeydown(event) { // editable input filed keyboard listener
    switch(event.key) {
      case 'ArrowUp': case 'Up': // select previous one
        this.setHighlighted(this.clearHighlighted(), -1);
        break;
      case 'ArrowDown': case 'Down': // select next one
        this.setHighlighted(this.clearHighlighted(), +1);
        break;
      case 'Enter':  // select current one
        this.select(this.ulEl.querySelector('[aria-selected=true]'));
      case 'Tab':
      case 'Escape': case 'Esc': // close openPopup
        this.closePopup();
        break;
    }
    event.key.match(/Up|Down|Enter|Esc/) && event.preventDefault();
  }

  onFocus(event) {
    const isExpanded = this.elRef.nativeElement.classList.contains('expanded');
    if (this.clearInput && !isExpanded) {
      this.inputEl.value = '';
      this.openPopup('');
    };
  }

  closePopup() { // when click/Enter/Tab, close popup
    this.inputEl.setAttribute('aria-expanded', 'false');
    this.elRef.nativeElement.classList.remove('expanded');
    this.clearHighlighted();
  }
  
  openPopup(text) { // when focus/input, open popup with matching text
    this.inputEl.setAttribute('aria-expanded', 'true');
    this.elRef.nativeElement.classList.add('expanded');

    let matchingEls = 0;
    if (text) { // show only text matching ones with value not empty
      matchingEls = Array.from(this.ulEl.children).reduce( (cnt, el:any) => {
        const matching = el.innerText.match(new RegExp(text, 'i'));
        const value = el.getAttribute('value');
        matching && value ? 
          el.removeAttribute('disabled') : el.setAttribute('disabled', '');
        return matching ? ++cnt : cnt;
      }, 0);
    } else { // if text is blank, show it all
      matchingEls = Array.from(this.ulEl.children).reduce( (cnt, el:any) => {
        el.removeAttribute('disabled');
        return ++cnt;
      }, 0);
    }
    this.clearHighlighted();
    if (matchingEls) { 
      this.ulEl.classList.remove('empty');
      this.setHighlighted(null, 1);
    } else { // if nothing selected, show 'Not Found'
      this.ulEl.classList.add('empty');
    }
  }

  private select(event: any) { // when click/Enter/Tab, select an option
    if (!event) return false;
    let text = (<HTMLLIElement>event.target || event).innerText;
    const obj = this.options.find(el => el.text === text);
    if (obj) {
      text=obj.value ? obj.text : obj.value;
      this.savedSelection = obj;
    }
    this.ngModel && this.ngModel.control.setValue(text);
    this.inputEl.value = text;
      
    if (!(event instanceof Event)) { //  move cursor to the end
      const range = document.createRange();
      range.selectNodeContents(this.inputEl);
      range.collapse(false);
      const sel = window.getSelection();
      sel.removeAllRanges();
      sel.addRange(range);
    }
  }

  private clearHighlighted() { // unset selected <li> in <ul>
    const highlightedEl = this.ulEl.querySelector('[aria-selected=true]');
    highlightedEl && highlightedEl.removeAttribute('id');
    highlightedEl && highlightedEl.setAttribute('aria-selected', 'false');
    return highlightedEl;
  }

  private setHighlighted(el, option) { // set selected <li> in <ul>
    const allAvailEls : Array<HTMLElement> = 
      Array.from(this.ulEl.querySelectorAll(':not([disabled])'));
    const curIndex = el ? allAvailEls.indexOf(el) : -1;
    const nxtIndex = 
      (curIndex + allAvailEls.length + option) % allAvailEls.length;
    allAvailEls[nxtIndex].setAttribute('id', `${this.id}-selected`);
    allAvailEls[nxtIndex].setAttribute('aria-selected', 'true');
  }

  private setElements() { // called by ngAfterViewInit, set attributes and events
    const thisEl = this.elRef.nativeElement;
    this.inputOrgEl = thisEl.querySelector('input:not(.editor)');
    this.ulEl = thisEl.querySelector('ul, select');
    this.inputEl = thisEl.querySelector('input.editor');
    if (this.inputOrgEl && this.ulEl && this.inputEl) {
      this.inputOrgEl.setAttribute('tabindex', '-1');

      this.ulEl.setAttribute('tabindex', '0');
      this.ulEl.setAttribute('role', 'listbox');
      this.ulEl.setAttribute('id', this.id);
      Array.from(this.ulEl.children).forEach(liEl => {
        liEl.setAttribute('role', 'option');
        liEl.setAttribute('aria-selected', 'false');
      });

      const placeholderEl: any = this.ulEl.querySelector('[value=""]');
      placeholderEl && 
        this.inputEl.setAttribute('placeholder',placeholderEl.innerText);
      `class,name,required,id,style.title,dir`.split(',').forEach(el => {
        const attrVal = this.inputOrgEl.getAttribute(el);
        if (attrVal !== null && el === 'class') {
          attrVal.replace(/ng-[a-z]+/g,'').split(' ').forEach(className => {
            className && this.inputEl.classList.add(className);
          });
        } else if (attrVal !== null) {
          el === 'id' && this.inputOrgEl.removeAttribute('id');
          this.inputEl.setAttribute(el, attrVal);
        }
      });
      this.inputEl.setAttribute('role', 'combobox')
      this.inputEl.setAttribute('aria-owns', this.id);
      this.inputEl.setAttribute('aria-expanded', 'false');
      this.inputEl.setAttribute('aria-activedescendant', `${this.id}-selected`);

      // set events for <li> click, <input> keydown
      Array.from(this.ulEl.children).forEach(liEl => {
        liEl.addEventListener('click', event => {
          this.select(event), this.inputEl.focus();
          this.closePopup();
        });
      });

      const selectedEl = this.ulEl.querySelector('[selected]');
      selectedEl && setTimeout(_ => this.select(selectedEl));

      return true;
    } else {
      console.error('Error on <combo-box>. Missing <input>/<ul>');
    }

  }

}
