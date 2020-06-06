#include "Utills.h"

namespace UHD {
    int ByteToInt(byte *_vals) {
        int val = 0;
        // for (int idx = 3; idx < TSKLEN; idx++) {
        //     val = (val << 8) + _vals[idx];
        // }
        
        return val;
    }
}