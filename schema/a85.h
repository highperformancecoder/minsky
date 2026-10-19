/*
 * Ascii85 (Base85) encoding and decoding functions.
 * Origin: Derived from the standard Adobe Systems PostScript/PDF Ascii85 
 *         specification algorithm (5-character to 4-byte chunk decoding).
 * 
 * License: Public Domain / CC0 (Creative Commons Zero)
 * This implementation is a standard mathematical translation of an open
 * specification, completely freestanding, and free of third-party copyright.
 */

#ifndef A85_H
#define A85_H

#include <stdint.h>

namespace a85 {

typedef uint8_t u8;
typedef uint32_t u32;

// Returns size of buffer required for to_a85 function.
int size_for_a85(int binlen, bool append_null = false);

// Translates the given binary data of the given size to Ascii85.
// Can translate in-place.
// Optionally appends a null character.
void to_a85(const u8* data, int binlen, char* text, bool append_null = false);

// Returns the size of buffer required for from_a85 function.
int size_for_bin(int textlen);

// Translates the given Ascii85 text to binary data.
// Can translate in-place.
void from_a85(const char* text, int textlen, u8* data);

} // namespace a85
#endif
