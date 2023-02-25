export class ScaleHandler {
    public get zoomFactor() {
        return this._zoomFactor;
    }

    // keep track of scale and zoom factors separately, because using zoom exclusively creates rounding errors
    constructor(private scaleBase = 1.1, private scalePower = 1, private scaleStep = 1 / 53, private _zoomFactor = 1) {
    }

    changeScale(delta: number) {
        this.scalePower -= delta * this.scaleStep;

        this._zoomFactor = Math.pow(this.scaleBase, this.scalePower);
    }
}