#include "utils.h"

int signf(float f)
{
    if (f > 0) return 1;
    if (f < 0) return -1;
    return 0;
}