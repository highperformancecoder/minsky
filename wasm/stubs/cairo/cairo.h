/*
  Minimal stand-in for cairo.h for the emscripten build, which has no
  cairo. It declares just the types and functions that Minsky's and
  EcoLab's headers mention, so that code needing only the *types* (eg
  ecolab::cairo::Surface, or the cairo_t* arguments of ecolab::Plot)
  compiles. All drawing is done through mansoura::IMansoura, which has
  an HTML canvas implementation. The functions are defined in
  cairoStub.cc: they do nothing useful and report an error if reached.
*/
#ifndef WASM_STUB_CAIRO_H
#define WASM_STUB_CAIRO_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo_pattern cairo_pattern_t;

typedef struct {double xx, yx, xy, yy, x0, y0;} cairo_matrix_t;

typedef enum {CAIRO_STATUS_SUCCESS=0} cairo_status_t;

typedef enum {
  CAIRO_SURFACE_TYPE_IMAGE,
  CAIRO_SURFACE_TYPE_RECORDING,
  CAIRO_SURFACE_TYPE_OTHER
} cairo_surface_type_t;

typedef struct {int status; void* data; int num_data;} cairo_path_t;

typedef struct {
  double x_bearing, y_bearing, width, height, x_advance, y_advance;
} cairo_text_extents_t;

#define CAIRO_HAS_RECORDING_SURFACE 1

cairo_t* cairo_create(cairo_surface_t*);
void cairo_destroy(cairo_t*);
void cairo_save(cairo_t*);
void cairo_restore(cairo_t*);
cairo_status_t cairo_status(cairo_t*);
const char* cairo_status_to_string(cairo_status_t);

void cairo_surface_destroy(cairo_surface_t*);
void cairo_surface_flush(cairo_surface_t*);
cairo_surface_type_t cairo_surface_get_type(cairo_surface_t*);
int cairo_image_surface_get_width(cairo_surface_t*);
int cairo_image_surface_get_height(cairo_surface_t*);
void cairo_recording_surface_ink_extents(cairo_surface_t*, double*, double*, double*, double*);

cairo_path_t* cairo_copy_path(cairo_t*);
void cairo_path_destroy(cairo_path_t*);
void cairo_append_path(cairo_t*, const cairo_path_t*);

void cairo_get_matrix(cairo_t*, cairo_matrix_t*);
void cairo_set_matrix(cairo_t*, const cairo_matrix_t*);
void cairo_matrix_translate(cairo_matrix_t*, double, double);
void cairo_identity_matrix(cairo_t*);
void cairo_translate(cairo_t*, double, double);
void cairo_scale(cairo_t*, double, double);
void cairo_rotate(cairo_t*, double);
void cairo_user_to_device_distance(cairo_t*, double*, double*);

void cairo_set_line_width(cairo_t*, double);
void cairo_set_dash(cairo_t*, const double*, int, double);
void cairo_set_source_rgb(cairo_t*, double, double, double);
void cairo_set_source_rgba(cairo_t*, double, double, double, double);
void cairo_set_source(cairo_t*, cairo_pattern_t*);
void cairo_move_to(cairo_t*, double, double);
void cairo_line_to(cairo_t*, double, double);
void cairo_rel_move_to(cairo_t*, double, double);
void cairo_rel_line_to(cairo_t*, double, double);
void cairo_arc(cairo_t*, double, double, double, double, double);
void cairo_rectangle(cairo_t*, double, double, double, double);
void cairo_new_path(cairo_t*);
void cairo_close_path(cairo_t*);
void cairo_fill(cairo_t*);
void cairo_clip(cairo_t*);
void cairo_stroke(cairo_t*);
void cairo_stroke_preserve(cairo_t*);
void cairo_show_text(cairo_t*, const char*);
void cairo_text_extents(cairo_t*, const char*, cairo_text_extents_t*);

cairo_pattern_t* cairo_pattern_create_for_surface(cairo_surface_t*);
void cairo_pattern_destroy(cairo_pattern_t*);
void cairo_pattern_get_matrix(cairo_pattern_t*, cairo_matrix_t*);
void cairo_pattern_set_matrix(cairo_pattern_t*, const cairo_matrix_t*);

#ifdef __cplusplus
}
#endif
#endif
