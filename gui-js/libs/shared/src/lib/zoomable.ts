export class Zoomable {
  zoomFactor=1;

  onKeyDown = async (event: KeyboardEvent) => {
    switch (event.key) {
    case '+': case 'numplus':
      (event.ctrlKey||event.metaKey) && this.zoom(1.1);
      break;
    case '-': case 'numsub':
      (event.ctrlKey||event.metaKey) && this.zoom(1/1.1);
      break;
    }
  }
  
  zoom(ratio: number) {
    this.zoomFactor*=ratio;
    document.body.style.setProperty('zoom', `${Math.round(this.zoomFactor*100)}%`);
  }
}

