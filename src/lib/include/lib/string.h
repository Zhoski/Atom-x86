#ifndef __STRING__
#define __STRING__

#include <stdint.h>

static inline uint32_t strlen(const uint8_t s[]) {
    uint32_t i = 0;
    while(s[i] != '\0')
        i++;

    return i;
}

static inline uint8_t strcmp(const uint8_t *s1, const uint8_t *s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

#endif
