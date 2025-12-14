#include "EDC.h"
#include <random>

void flip_random_bit(uint32_t* ptr, size_t length)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, length * 32 - 1);
    int position = dis(gen);
    
    int array_index = position / 32;
    int bit_position = position % 32;
    ptr[array_index] ^= (1u << bit_position);
}