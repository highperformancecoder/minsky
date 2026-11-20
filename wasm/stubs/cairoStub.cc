// Definitions of the cairo functions declared in cairo/cairo.h. The
// wasm build has no cairo: reaching one of these is a bug, so report it.
#include <cairo/cairo.h>
#include <stdexcept>
#include <string>

namespace
{
  [[noreturn]] void unsupported(const char* fn)
  {throw std::runtime_error(std::string(fn)+": cairo is not available in the wasm build");}
}
#define NOCAIRO(name) unsupported(#name)
extern "C" {
cairo_t* cairo_create(cairo_surface_t* s) {if (s) NOCAIRO(cairo_create); return nullptr;}
// destructors of objects holding null handles must be harmless
void cairo_destroy(cairo_t*) {}
void cairo_surface_destroy(cairo_surface_t*) {}
void cairo_save(cairo_t*) {NOCAIRO(cairo_save);}
void cairo_restore(cairo_t*) {NOCAIRO(cairo_restore);}
cairo_status_t cairo_status(cairo_t*) {return CAIRO_STATUS_SUCCESS;}
const char* cairo_status_to_string(cairo_status_t) {return "success";}
void cairo_surface_flush(cairo_surface_t*) {NOCAIRO(cairo_surface_flush);}
cairo_surface_type_t cairo_surface_get_type(cairo_surface_t*) {return CAIRO_SURFACE_TYPE_OTHER;}
int cairo_image_surface_get_width(cairo_surface_t*) {NOCAIRO(cairo_image_surface_get_width);}
int cairo_image_surface_get_height(cairo_surface_t*) {NOCAIRO(cairo_image_surface_get_height);}
void cairo_recording_surface_ink_extents(cairo_surface_t*, double*, double*, double*, double*) {NOCAIRO(cairo_recording_surface_ink_extents);}
cairo_path_t* cairo_copy_path(cairo_t*) {NOCAIRO(cairo_copy_path);}
void cairo_path_destroy(cairo_path_t*) {}
void cairo_append_path(cairo_t*, const cairo_path_t*) {NOCAIRO(cairo_append_path);}
void cairo_get_matrix(cairo_t*, cairo_matrix_t*) {NOCAIRO(cairo_get_matrix);}
void cairo_set_matrix(cairo_t*, const cairo_matrix_t*) {NOCAIRO(cairo_set_matrix);}
void cairo_matrix_translate(cairo_matrix_t* m, double tx, double ty)
{m->x0+=m->xx*tx+m->xy*ty; m->y0+=m->yx*tx+m->yy*ty;}
void cairo_identity_matrix(cairo_t*) {NOCAIRO(cairo_identity_matrix);}
void cairo_translate(cairo_t*, double, double) {NOCAIRO(cairo_translate);}
void cairo_scale(cairo_t*, double, double) {NOCAIRO(cairo_scale);}
void cairo_rotate(cairo_t*, double) {NOCAIRO(cairo_rotate);}
void cairo_user_to_device_distance(cairo_t*, double*, double*) {NOCAIRO(cairo_user_to_device_distance);}
void cairo_set_line_width(cairo_t*, double) {NOCAIRO(cairo_set_line_width);}
void cairo_set_dash(cairo_t*, const double*, int, double) {NOCAIRO(cairo_set_dash);}
void cairo_set_source_rgb(cairo_t*, double, double, double) {NOCAIRO(cairo_set_source_rgb);}
void cairo_set_source_rgba(cairo_t*, double, double, double, double) {NOCAIRO(cairo_set_source_rgba);}
void cairo_set_source(cairo_t*, cairo_pattern_t*) {NOCAIRO(cairo_set_source);}
void cairo_move_to(cairo_t*, double, double) {NOCAIRO(cairo_move_to);}
void cairo_line_to(cairo_t*, double, double) {NOCAIRO(cairo_line_to);}
void cairo_rel_move_to(cairo_t*, double, double) {NOCAIRO(cairo_rel_move_to);}
void cairo_rel_line_to(cairo_t*, double, double) {NOCAIRO(cairo_rel_line_to);}
void cairo_arc(cairo_t*, double, double, double, double, double) {NOCAIRO(cairo_arc);}
void cairo_rectangle(cairo_t*, double, double, double, double) {NOCAIRO(cairo_rectangle);}
void cairo_new_path(cairo_t*) {NOCAIRO(cairo_new_path);}
void cairo_close_path(cairo_t*) {NOCAIRO(cairo_close_path);}
void cairo_fill(cairo_t*) {NOCAIRO(cairo_fill);}
void cairo_clip(cairo_t*) {NOCAIRO(cairo_clip);}
void cairo_stroke(cairo_t*) {NOCAIRO(cairo_stroke);}
void cairo_stroke_preserve(cairo_t*) {NOCAIRO(cairo_stroke_preserve);}
void cairo_show_text(cairo_t*, const char*) {NOCAIRO(cairo_show_text);}
void cairo_text_extents(cairo_t*, const char*, cairo_text_extents_t*) {NOCAIRO(cairo_text_extents);}
cairo_pattern_t* cairo_pattern_create_for_surface(cairo_surface_t*) {NOCAIRO(cairo_pattern_create_for_surface);}
void cairo_pattern_destroy(cairo_pattern_t*) {}
void cairo_pattern_get_matrix(cairo_pattern_t*, cairo_matrix_t*) {NOCAIRO(cairo_pattern_get_matrix);}
void cairo_pattern_set_matrix(cairo_pattern_t*, const cairo_matrix_t*) {NOCAIRO(cairo_pattern_set_matrix);}
}

