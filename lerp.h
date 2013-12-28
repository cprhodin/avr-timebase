#ifndef _LERP_H_
#define _LERP_H_

struct interpolant {
    uint16_t x;     // x0
    uint16_t dx;    // x1 - x0
    uint16_t y;     // y0
    uint16_t dy;    // y1 - y0
};


uint16_t lerp(uint16_t x, struct interpolant * itpt);

#endif // _LERP_H_
