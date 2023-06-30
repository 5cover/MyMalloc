#ifndef BITMAPFACTORY_H_INCLUDED
#define BITMAPFACTORY_H_INCLUDED

#include <stdint.h>

#define BYTES_PER_PIXEL 3 // red, green, & blue

enum
{
    I_R = 2,
    I_G = 1,
    I_B = 0,
};

void generateBitmapImage(uint8_t const *image, uint32_t height, uint32_t width, char const *imageFileName);

#endif // BITMAPFACTORY_H_INCLUDED
