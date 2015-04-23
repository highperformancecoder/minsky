
/**
 *  @namespace minsky
 */

var minsky = minksy || { version: "0,1" };

/**
 *  @class minksy.object
 *
 *  Base class for minsky widgets.
 */
minsky.Object = fabric.util.createClass(fabric.Object, {

  type: 'minsky-object',   ///< type of this object.

  /**
   *  Initializes the object.
   *
   *  @param options Initializers.
   */
  initialize: function (options) {
    options || (options = { });
    this.set('hasRotatingPoint', options.hasRotatingPoint || false);
    this.set('includeDefaultValues', options.includeDefaultValues || false);
    this.set('selectable', options.selectable || true);
    this.set('visible', options.visible || true);
  },
  /**
   *  Draws the object onto the canvas.
   *
   *  Objects of this type are invisible, this method simply calls 
   *  fabric.Obect._render
   *
   *  @param ctx  The context to use for painting.
   */
    _render: function (ctx) {
    this.callSuper('_render', ctx);
  }
});

minksy.Operation = fabric.util.createClass(MinskyObject, {

  type: 'minsky-operation',

  initialize: function (options) {

    options || (options = {});

    this.callSuper('initialize', options);
    this.set('label', options.label || '');
    this.set('modelID', options.modelID || '');
  },

  _render: function (ctx) {
    this.callSuper('_render', ctx);
  }
});

/**
 *  @class MinskyLink
 *  
 *  Basic link between two nodes.
 */
var MinkyLink = fabric.createClass(MinskyObject, {

  type: 'minksyLink',
  line: fabric.line,

  initialize: function (options) {

    options || (options = {});

    this.callSuper(options);
    line.initialize({ x1: options.x1, y1: options.y1 });
  },

  _render: function (ctx) {

  line.call('_render', ctx);

  }
});

