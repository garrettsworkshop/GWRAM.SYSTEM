#include <stdint.h>

#ifndef UTIL_H
#define UTIL_H

#define true 1
#define false 0

#define VBL ((signed char*)0xC019)

char read_applekey(void);
void wait(char frames);
void spin(uint8_t x, uint8_t y);

#endif /* UTIL_H */