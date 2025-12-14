#include "EDC.h"
#include <immintrin.h>

namespace Linear
{
    uint32_t Linear(uint32_t* ptr, size_t length)
    {
        uint64_t result = 0;
        for (size_t i = 0; i < length; i++)
            result += ptr[i];
        result = result % 0xFFFFFFFF;
        return (uint32_t)result;
    }
    uint32_t Linear_SIMD(uint32_t* ptr, size_t length)
    {
        __m256i vsum = _mm256_setzero_si256();
        size_t i = 0;

        for (; i + 8 <= length; i += 8) {
            __m256i vdata = _mm256_loadu_si256((__m256i*)&ptr[i]);
            vsum = _mm256_add_epi32(vsum, vdata);
        }

        __m128i sum128 = _mm_add_epi32(
            _mm256_castsi256_si128(vsum),
            _mm256_extracti128_si256(vsum, 1)
        );
        sum128 = _mm_add_epi32(sum128, _mm_srli_si128(sum128, 8));
        sum128 = _mm_add_epi32(sum128, _mm_srli_si128(sum128, 4));

        uint32_t result = _mm_cvtsi128_si32(sum128);

        for (; i < length; i++)
            result += ptr[i];
        return result % 0xFFFFFFFF;
    }

    void run_perf_test(uint32_t (*func)(uint32_t*, size_t), size_t length, uint32_t* A)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < PERF_COUNT; i++)
            func(A + i * length, length);       // for non-cache test
            // func(A, length);             // for cache test
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double volume = length * sizeof(uint32_t) * PERF_COUNT / 1024.0 / 1024.0 / 1024.0; // GB
        printf("%f GB/s\n", (double)volume / duration.count() * 1000.0);
        return;
    }
    void unit_test()
    {
        printf("\n-----Linear unit test-----\n");
        printf("Do nothing.\n");
        size_t length = max_len;
        uint32_t* A = (uint32_t*)malloc(length * sizeof(uint32_t) * PERF_COUNT);
        for (size_t i = 0; i < length; i++)
            A[i] = i;
            
        uint32_t result1 = Linear(A, length);
        uint32_t result2 = Linear_SIMD(A, length);
        if (result1 == result2)
            printf("Functional: correct! %u %u\n", result1, result2);

        printf("Linear: ");
        run_perf_test(Linear, length, A);
        printf("Linear SIMD: ");
        run_perf_test(Linear_SIMD, length, A);

        return;
    }
}
