// a85.cpp

#include "a85.h"
#include <assert.h>

namespace a85 {

int size_for_a85(int binlen, bool append_null) {
    return (binlen * 5 + 3) / 4 + !!append_null;
}

void to_a85(const u8* data, int binlen, char* text, bool append_null) {
    data += binlen;
    text += size_for_a85(binlen,append_null);
    if (append_null) {
        *(--text) = 0;
    }
    const int rem = binlen & 3;
    if (rem) {
        u32 val = 0;
        for (int i = 4 - rem; i < 4; i++) {
            val |= *(--data) << (8 * i);
        }
        int i;
        for (i = 0; i < 4 - rem; i++) {
            val /= 85;
        }
        for (; i <= 4; i++) {
            *(--text) = val % 85 + 33;
            val /= 85;
        }
        binlen &= ~3;
    }
    while (binlen) {
        u32 val = *(--data);
        val |= *(--data) << 8;
        val |= *(--data) << 16;
        val |= *(--data) << 24;
        *(--text) = val % 85 + 33;
        val /= 85;
        *(--text) = val % 85 + 33;
        val /= 85;
        *(--text) = val % 85 + 33;
        val /= 85;
        *(--text) = val % 85 + 33;
        val /= 85;
        *(--text) = val % 85 + 33;
        binlen -= 4;
    }
}

int size_for_bin(int textlen) {
    return (textlen * 4) / 5;
}

void from_a85(const char* text, int textlen, u8* data) {
#ifndef NDEBUG
  int binLen=size_for_bin(textlen);
#endif
    while (textlen) {
        if (textlen < 5) {
            u32 val = 0;
            unsigned factor = 52200625;
            int i;
            for (i = 0; i < textlen; i++) {
                val += (*(text++) - 33) * factor;
                factor /= 85;
            }
            for (; i < 5; i++) {
                val += 'u' * factor;
                factor /= 85;
            }
            int shift = 24;
            for (i = 0; i < textlen - 1; i++) {
                *(data++) = val >> shift;
                shift -= 8;
            }
            break;
        }
        u32 val = (*(text++) - 33) * 52200625U;
        val += (*(text++) - 33) * 614125;
        val += (*(text++) - 33) * 7225;
        val += (*(text++) - 33) * 85;
        val += (*(text++) - 33);
        assert((binLen-=4)>=0);
        *(data++) = val >> 24;
        *(data++) = val >> 16;
        *(data++) = val >> 8;
        *(data++) = val;
        textlen -= 5;
    }
}

} // namespace a85
