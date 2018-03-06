/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/
#ifndef CAPIRENDERER_H
#define CAPIRENDERER_H
// Renderer interface. See CairoRenderer for implementation and
// semantics WRT to the Cairo graphics library
struct CAPIRenderer
{
  void (*moveTo)(CAPIRenderer*, double x, double y);
  void (*lineTo)(CAPIRenderer*, double x, double y);
  void (*relMoveTo)(CAPIRenderer*, double x, double y);
  void (*relLineTo)(CAPIRenderer*, double x, double y);
  void (*arc)(CAPIRenderer*, double x, double y,
              double radius, double start, double end);

  void (*setLineWidth)(CAPIRenderer*,double);

  // paths
  void (*newPath)(CAPIRenderer*);
  void (*closePath)(CAPIRenderer*);
  void (*fill)(CAPIRenderer*);
  void (*stroke)(CAPIRenderer*);
  void (*strokePreserve)(CAPIRenderer*);
  
  // sources
  void (*setSourceRGB)(CAPIRenderer*, double r, double g, double b);
    
  // text. Argument is in UTF8 encoding
  void (*showText)(CAPIRenderer*, const char*);
  void (*setTextExtents)(CAPIRenderer*, const char*);
  double (*textWidth)(CAPIRenderer*);
  double (*textHeight)(CAPIRenderer*);

  // matrix transformation
  void (*identityMatrix)(CAPIRenderer*);
  void (*translate)(CAPIRenderer*, double x, double y);
  void (*scale)(CAPIRenderer*, double sx, double sy);
  void (*rotate)(CAPIRenderer*, double angle); ///< angle in radians

        // context manipulation
  void (*save)(CAPIRenderer*);
  void (*restore)(CAPIRenderer*);
};
#endif
