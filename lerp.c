#include "project.h"

#include "mathops.h"
#include "lerp.h"

uint16_t lerp(uint16_t x, struct interpolant * itpt)
{
    return itpt->y + _ummdr(x - itpt->x, itpt->dy, itpt->dx);
}
