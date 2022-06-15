"use strict";
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.VariablePaneComponent = void 0;
var core_1 = require("@angular/core");
var shared_1 = require("@minsky/shared");
var ngx_auto_unsubscribe_1 = require("ngx-auto-unsubscribe");
var rxjs_1 = require("rxjs");
var operators_1 = require("rxjs/operators");
var VariablePaneComponent = /** @class */ (function () {
    function VariablePaneComponent(communicationService, windowUtilityService, electronService, route) {
        var _this = this;
        this.communicationService = communicationService;
        this.windowUtilityService = windowUtilityService;
        this.electronService = electronService;
        this.route = route;
        this.leftOffset = 0;
        this.mouseX = 0;
        this.mouseY = 0;
        this.onKeyDown = function (event) { return __awaiter(_this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        console.log(shared_1.green("godley window "), event.key.charCodeAt(0));
                        return [4 /*yield*/, this.communicationService.handleKeyDown({
                                event: event,
                                command: this.namedItemSubCommand,
                            })];
                    case 1:
                        _a.sent();
                        return [4 /*yield*/, this.redraw()];
                    case 2:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        }); };
        this.onMouseWheelZoom = function (event) { return __awaiter(_this, void 0, void 0, function () {
            var deltaY, zoomIn, command, zoomFactor, _a, x, y;
            return __generator(this, function (_b) {
                switch (_b.label) {
                    case 0:
                        event.preventDefault();
                        deltaY = event.deltaY;
                        zoomIn = deltaY < 0;
                        command = "/minsky/variablePane/zoom";
                        zoomFactor = zoomIn ? shared_1.ZOOM_IN_FACTOR : shared_1.ZOOM_OUT_FACTOR;
                        _a = this.electronService.remote.getCurrentWindow().getContentSize(), x = _a[0], y = _a[1];
                        //TODO: throttle here if required
                        return [4 /*yield*/, this.electronService.sendMinskyCommandAndRender({
                                command: command + " [" + x / 2 + "," + y / 2 + "," + zoomFactor + "]",
                            })];
                    case 1:
                        //TODO: throttle here if required
                        _b.sent();
                        return [2 /*return*/];
                }
            });
        }); };
        this.route.queryParams.subscribe(function (params) {
            _this.itemId = params.itemId;
            _this.systemWindowId = params.systemWindowId;
        });
    }
    VariablePaneComponent.prototype.ngAfterViewInit = function () {
        this.namedItemSubCommand = "/minsky/variablePane";
        this.getWindowRectInfo();
        this.renderFrame();
        this.initEvents();
    };
    VariablePaneComponent.prototype.windowResize = function () {
        this.getWindowRectInfo();
        this.renderFrame();
    };
    VariablePaneComponent.prototype.getWindowRectInfo = function () {
        this.godleyCanvasContainer = this.godleyCanvasElemWrapper
            .nativeElement;
        var clientRect = this.godleyCanvasContainer.getBoundingClientRect();
        this.leftOffset = Math.round(clientRect.left);
        this.topOffset = Math.round(this.windowUtilityService.getElectronMenuBarHeight());
        this.height = Math.round(this.godleyCanvasContainer.clientHeight);
        this.width = Math.round(this.godleyCanvasContainer.clientWidth);
    };
    VariablePaneComponent.prototype.renderFrame = function () {
        if (this.electronService.isElectron &&
            this.systemWindowId &&
            this.itemId &&
            this.height &&
            this.width) {
            var scaleFactor = this.electronService.remote.screen.getPrimaryDisplay()
                .scaleFactor;
            var command = this.namedItemSubCommand + "/renderFrame [" + this.systemWindowId + "," + this.leftOffset + "," + this.topOffset + "," + this.width + "," + this.height + "," + scaleFactor + "]";
            this.electronService.sendMinskyCommandAndRender({
                command: command,
            });
        }
    };
    VariablePaneComponent.prototype.initEvents = function () {
        var _this = this;
        this.variablePaneContainer.addEventListener('scroll', function () { return __awaiter(_this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.handleScroll(this.godleyCanvasContainer.scrollTop, this.godleyCanvasContainer.scrollLeft)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        }); });
        this.mouseMove$ = rxjs_1.fromEvent(this.variablePaneContainer, 'mousemove').pipe(operators_1.sampleTime(1)); /// FPS=1000/sampleTime
        this.mouseMove$.subscribe(function (event) { return __awaiter(_this, void 0, void 0, function () {
            var clientX, clientY;
            return __generator(this, function (_a) {
                clientX = event.clientX, clientY = event.clientY;
                this.mouseX = clientX;
                this.mouseY = clientY;
                this.sendMouseEvent(clientX, clientY, shared_1.commandsMapping.MOUSEMOVE_SUBCOMMAND);
                return [2 /*return*/];
            });
        }); });
        this.godleyCanvasContainer.addEventListener('mousedown', function (event) {
            var clientX = event.clientX, clientY = event.clientY;
            _this.sendMouseEvent(clientX, clientY, shared_1.commandsMapping.MOUSEDOWN_SUBCOMMAND);
        });
        this.variablePaneContainer.addEventListener('mouseup', function (event) { return __awaiter(_this, void 0, void 0, function () {
            var clientX, clientY;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        clientX = event.clientX, clientY = event.clientY;
                        return [4 /*yield*/, this.sendMouseEvent(clientX, clientY, shared_1.commandsMapping.MOUSEUP_SUBCOMMAND)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        }); });
        this.variablePaneContainer.onwheel = this.onMouseWheelZoom;
        document.onkeydown = this.onKeyDown;
    };
    VariablePaneComponent.prototype.redraw = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.electronService.sendMinskyCommandAndRender({
                            command: this.namedItemSubCommand + "/requestRedraw",
                        })];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    VariablePaneComponent.prototype.sendMouseEvent = function (x, y, type) {
        return __awaiter(this, void 0, void 0, function () {
            var command;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        command = this.namedItemSubCommand + "/" + type + " [" + x + "," + y + "]";
                        return [4 /*yield*/, this.electronService.sendMinskyCommandAndRender({
                                command: command,
                            })];
                    case 1:
                        _a.sent();
                        return [4 /*yield*/, this.redraw()];
                    case 2:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    //  async handleScroll(scrollTop: number, scrollLeft: number) {
    //    //TODO: throttle here if required
    //
    //    const cols = (await this.electronService.sendMinskyCommandAndRender({
    //      command: `${commandsMapping.GET_NAMED_ITEM}/"${this.itemId}"/second/table/cols`,
    //    })) as number;
    //
    //    const rows = (await this.electronService.sendMinskyCommandAndRender({
    //      command: `${commandsMapping.GET_NAMED_ITEM}/"${this.itemId}"/second/table/rows`,
    //    })) as number;
    //
    //    const stepX = this.godleyCanvasContainer.scrollHeight / cols;
    //    const stepY = this.godleyCanvasContainer.scrollHeight / rows;
    //
    //    const currentStepX = Math.round(scrollLeft / stepX);
    //    const currentStepY = Math.round(scrollTop / stepY);
    //
    //    await this.electronService.sendMinskyCommandAndRender({
    //      command: `${this.namedItemSubCommand}/scrollColStart ${currentStepX}`,
    //    });
    //
    //    await this.electronService.sendMinskyCommandAndRender({
    //      command: `${this.namedItemSubCommand}/scrollRowStart ${currentStepY}`,
    //    });
    //
    //    this.redraw();
    //  }
    // eslint-disable-next-line @typescript-eslint/no-empty-function,@angular-eslint/no-empty-lifecycle-method
    VariablePaneComponent.prototype.ngOnDestroy = function () { };
    __decorate([
        core_1.ViewChild('godleyCanvasElemWrapper')
    ], VariablePaneComponent.prototype, "godleyCanvasElemWrapper", void 0);
    VariablePaneComponent = __decorate([
        ngx_auto_unsubscribe_1.AutoUnsubscribe(),
        core_1.Component({
            selector: 'minsky-variable-pane',
            templateUrl: './variable-pane.component.html',
            styleUrls: ['./variable-pane.component.scss'],
        })
    ], VariablePaneComponent);
    return VariablePaneComponent;
}());
exports.VariablePaneComponent = VariablePaneComponent;
