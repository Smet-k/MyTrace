#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>

struct Options {
    char* destination;
    uint8_t maxTTL;
    uint8_t timeout;
};

void trace(struct Options options);

#endif