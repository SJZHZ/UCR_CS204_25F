#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <chrono>
#include <cmath>



#define BITS_BYTE 8
#define WORD_SIZE 32

#define MAX_LEN 1000     // 32 bits per word

extern unsigned int PERF_COUNT;  // approximately 1 GB
extern size_t max_len;


struct Block
{
    uint32_t* data;
    size_t length;
};

void flip_random_bit(uint32_t*, size_t);


namespace CRC
{
    void unit_test();
}

namespace Linear
{
    void unit_test();
}

namespace BigMod
{
    void unit_test();
}
