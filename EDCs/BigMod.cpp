#include "EDC.h"

#include <immintrin.h>      // SIMD
#include <cstdint>

#define BIGMOD_HD_MAX 5

// #define BIGMOD_BITS 28
// #define BIGMOD_LOOP ((int)8)    // 1 << (32 - 28 - 1) = 8
// #define BIGMOD_MODULI 0b1111010111110000000010100111

#define BIGMOD_BITS 24
#define BIGMOD_LOOP ((int)128)    // 1 << (32 - 24 - 1) = 128
#define BIGMOD_MODULI 0b111111110000000010100111   // 16711847 is prime

// #define BIGMOD_BITS 20
// #define BIGMOD_LOOP ((int)2048)    // 1 << (32 - 20 - 1) = 2048
// #define BIGMOD_MODULI 1047373

#define BIGMOD_ITERATION 64

void SIMD_test()        // SIMD multiplication
{
    uint64_t result_SIMD = 0, result = 0;
    uint32_t src_v32_arr[4] = {1, 2, 3, 4};
    uint64_t mul_v64_arr[4] = {5, 6, 7, 8};
    uint64_t sum_v64_arr[4] = {0, 0, 0, 0}; // to store the result of the 256-bit vector
    __m128i src_v32 = _mm_loadu_si128((__m128i*)src_v32_arr);  // load 4 32-bit integers
    __m256i src_v64 = _mm256_cvtepu32_epi64(src_v32);          // convert to 4 64-bit integers
    __m256i mul_v64 = _mm256_loadu_si256((__m256i*)mul_v64_arr);  // load 4 64-bit integers
    __m256i result_v64 = _mm256_mul_epu32(src_v64, mul_v64);  // multiply with the vector table

    sum_v64_arr[0] = _mm256_extract_epi64(result_v64, 0);
    sum_v64_arr[1] = _mm256_extract_epi64(result_v64, 1);
    sum_v64_arr[2] = _mm256_extract_epi64(result_v64, 2);
    sum_v64_arr[3] = _mm256_extract_epi64(result_v64, 3);

    for (int i = 0; i < 4; i++)
        result += src_v32_arr[i] * mul_v64_arr[i];  // naive multiplication
    for (int i = 0; i < 4; i++)
        result_SIMD += sum_v64_arr[i];  // accumulate the result
    printf("\nSIMD multiplication test: \n");
    printf("  Result: %lu %lu, ", result_SIMD, result);
    if (result_SIMD == result)
        printf("correct.\n");
    else 
        printf("incorrect!\n");
    return;
}


namespace BigMod
{
    struct Error    // error value -> bit index
    {
        uint32_t value;
        int32_t index;
    };
    uint64_t BigMod_MagicNumber[2][2];
    size_t code_length;         // 16 ~ 28
    
    uint64_t* Vector_Table; //[MAX_LEN + BIGMOD_LOOP];       // word index -> error value, 32 bits enough but 64 bits for fast MUL
    uint64_t* Vector_Table_Iterative; //[BIGMOD_ITERATION + BIGMOD_LOOP];
    uint64_t Vector_Table_Iterative_Tail;

    uint32_t*** Error_Table; //[2][MAX_LEN * 32][2];           // bit index -> error value, negative and positive

    uint32_t moduli[2] = {BIGMOD_MODULI, BIGMOD_MODULI};
    uint32_t HD[BIGMOD_HD_MAX] = {0};
    size_t Combination[BIGMOD_HD_MAX] = {0};

    void InitLUT(size_t length)         // Vector Table
    {
        Vector_Table = (uint64_t*)malloc((length + BIGMOD_LOOP) * sizeof(uint64_t));
        Vector_Table_Iterative = (uint64_t*)malloc((BIGMOD_ITERATION + BIGMOD_LOOP) * sizeof(uint64_t));

        uint32_t modulus = moduli[0];
        uint64_t temp = 1;
        for (int i = length - 1; i >= 0; i--)
        {
            temp <<= 32;
            temp %= modulus;
            Vector_Table[i] = temp;         // BigMod
        }
        
        modulus = moduli[1];
        temp = 1;
        for (int i = length - 1; i >= 0; i--)
        {
            temp <<= 32;
            temp %= modulus;
        }

        uint64_t temptemp = 1;
            for (int i = BIGMOD_ITERATION - 1; i >= 0; i--)
            {
                temptemp <<= 32;
                temptemp %= moduli[0];
                Vector_Table_Iterative[i] = temptemp;
            }
        Vector_Table_Iterative_Tail = (temptemp << 32) % moduli[0];     // as if result at pos -1
        return;
    }
    void InitErrorTable(size_t length)  // Error Table and Reverse Error Table
    {
        Error_Table = (uint32_t***)malloc(2 * sizeof(uint32_t**));
        for (int p = 0; p < 2; p++)
        {
            Error_Table[p] = (uint32_t**)malloc((length * 32) * sizeof(uint32_t*));
            for (size_t i = 0; i < length * 32; i++)
                Error_Table[p][i] = (uint32_t*)malloc(2 * sizeof(uint32_t));
        }
        for (int p = 0; p < 2; p++)
        {
            uint32_t modulus = moduli[p];
            uint32_t temp = (1ull << 32) % modulus;
            for (int32_t i = (int32_t)length * 32 - 1; i >= 0; i--) // start from rightmost bit
            {
                temp %= modulus;
                Error_Table[p][i][0] = temp;
                Error_Table[p][i][1] = modulus - temp;
                temp <<= 1;
            }
        }
        return;
    }
    void InitMagicNumber()
    {
        for (int p = 0; p < 2; p++)         // Guarantee no overflow within a loop
        {
            uint64_t temp = (1ull << 63) + moduli[p];
            temp -= temp % moduli[p];   // make magic number a multiple of moduli
            BigMod_MagicNumber[p][0] = temp;
        }
        for (int p = 0; p < 2; p++)         // Guarantee no overflow within a SIMD addition
        {
            uint64_t temp = (1ull << 62) + moduli[p];
            temp -= temp % moduli[p];
            BigMod_MagicNumber[p][1] = temp;
        }
        printf("BigMod Magic Number(LOOP): %lu %lu\n", BigMod_MagicNumber[0][0], BigMod_MagicNumber[1][0]);
        printf("BigMod Magic Number(FINAL): %lu %lu\n", BigMod_MagicNumber[0][1], BigMod_MagicNumber[1][1]);
        return;
    }


    uint32_t BigMod_Bitwise(uint32_t* ptr, size_t length)
    {
        uint64_t result = 0;
        for (size_t i = 0; i < length; i++)
        {
            result = (result << 32) + ptr[i];
            uint64_t temp = (uint64_t)BIGMOD_MODULI << 32;
            for (int j = 0; j < 33; j++)        // from 0 to 32
            {
                if (result >= temp)
                    result -= temp;
                temp >>= 1;
            }
        }
        result <<= 32;
        uint64_t temp = (uint64_t)BIGMOD_MODULI << 32;
        for (int j = 0; j < 33; j++)
        {
            if (result >= temp)
                result -= temp;
            temp >>= 1;
        }
        return (uint32_t)result;
    }
    uint32_t BigMod_Naive(uint32_t* ptr, size_t length)    // Koopman's checksum
    {
        uint64_t result = 0;
        for (size_t i = 0; i < length; i++)
            result = ((result << 32) + ptr[i]) % BIGMOD_MODULI;
        result <<= 32;
        result %= BIGMOD_MODULI;
        return (uint32_t)result;
    }
    uint32_t BigMod_Table(uint32_t* ptr, size_t length)
    {
        uint64_t result = 0;
        int len = (int)length;
        for (int i = 0; i <= len - BIGMOD_LOOP; i += BIGMOD_LOOP)
        {
            for (size_t j = 0; j < BIGMOD_LOOP; j++)            // after k times addition, the result might be 64 bits
                result += Vector_Table[i + j] * ptr[i + j];
            if (result >= BigMod_MagicNumber[0][0])                // avoid overflow
                result -= BigMod_MagicNumber[0][0];
        }
        for (int i = len - len % BIGMOD_LOOP; i < len; i++)
            result += Vector_Table[i] * ptr[i];
        result %= BIGMOD_MODULI;
        return (uint32_t)result;
    }
    uint32_t BigMod_Table_Iterative(uint32_t* ptr, size_t length)
    {
        uint64_t result = 0;
        int len = (int)length, i = 0;
        for (; i <= len - (BIGMOD_ITERATION + 1); i += BIGMOD_ITERATION + 1)     // % ITERATION
        {
            result *= Vector_Table_Iterative_Tail;
            for (size_t j = 0; j < BIGMOD_ITERATION; j++)
                result += Vector_Table_Iterative[j] * ptr[i + j];
            result += ptr[i + BIGMOD_ITERATION];        // coef = 1
            result %= BIGMOD_MODULI;
        }
        if (i == len)           // no more elements
            return (uint32_t)((result << 32) % BIGMOD_MODULI);
        if (i == len - BIGMOD_ITERATION)
        {
            result *= Vector_Table_Iterative_Tail;
            for (size_t j = 0; j < BIGMOD_ITERATION; j++)
                result += Vector_Table_Iterative[j] * ptr[i + j];
            result %= BIGMOD_MODULI;
            return (uint32_t)result;
        }
        int remain = len % (BIGMOD_ITERATION + 1);      // 1 ~ BIGMOD_ITERATION-1
        int bias = BIGMOD_ITERATION - remain;           // 1 -> BIGMOD_ITERATION-1, BIGMOD_ITERATION-1 -> 1
        result *= Vector_Table_Iterative[bias - 1];     // 1 -> BIGMOD_ITERATION-2, BIGMOD_ITERATION-1 -> 0

        for (int index = 0; index < remain; index++)
            result += Vector_Table_Iterative[bias + index] * ptr[i + index];
        result %= BIGMOD_MODULI;
        return (uint32_t)result;
    }
    uint32_t BigMod_Table_SIMD(uint32_t* ptr, size_t length)
    {
        uint64_t result = 0;
        int len = (int)length;
        __m256i sum_v64 = _mm256_setzero_si256();
        uint64_t sum_v64_arr[4] = {0, 0, 0, 0};                     // buffer for post-processing

        for (int i = 0; i <= len - BIGMOD_LOOP * 4; i += BIGMOD_LOOP * 4)    
        {
            for (size_t j = 0; j < BIGMOD_LOOP * 4; j += 4)
            {
                __m128i src_v32 = _mm_loadu_si128((__m128i*)&ptr[i + j]);
                __m256i src_v64 = _mm256_cvtepu32_epi64(src_v32);       // convert for SIMD multiplication
                __m256i mul_v64 = _mm256_loadu_si256((__m256i*)&Vector_Table[i + j]);
                __m256i result_v64 = _mm256_mul_epu32(src_v64, mul_v64);
                sum_v64 = _mm256_add_epi64(sum_v64, result_v64);
            }
            // post-processing
            sum_v64_arr[0] = _mm256_extract_epi64(sum_v64, 0);      // load for post-processing
            sum_v64_arr[1] = _mm256_extract_epi64(sum_v64, 1);
            sum_v64_arr[2] = _mm256_extract_epi64(sum_v64, 2);
            sum_v64_arr[3] = _mm256_extract_epi64(sum_v64, 3);
            for (int j = 0; j < 4; j++)
                if (sum_v64_arr[j] >= BigMod_MagicNumber[0][0])        // avoid overflow
                    sum_v64_arr[j] -= BigMod_MagicNumber[0][0];
            sum_v64 = _mm256_loadu_si256((__m256i*)sum_v64_arr);    // load back for next iteration
        }
        // remain elements (less than BIGMOD_LOOP * 4)
        for (int i = len - len % (BIGMOD_LOOP * 4); i <= len - 4; i += 4)
        {
            __m128i src_v32 = _mm_loadu_si128((__m128i*)&ptr[i]);
            __m256i src_v64 = _mm256_cvtepu32_epi64(src_v32);
            __m256i mul_v64 = _mm256_loadu_si256((__m256i*)&Vector_Table[i]);
            __m256i result_v64 = _mm256_mul_epu32(src_v64, mul_v64);
            sum_v64 = _mm256_add_epi64(sum_v64, result_v64);
        }
        sum_v64_arr[0] = _mm256_extract_epi64(sum_v64, 0);
        sum_v64_arr[1] = _mm256_extract_epi64(sum_v64, 1);
        sum_v64_arr[2] = _mm256_extract_epi64(sum_v64, 2);
        sum_v64_arr[3] = _mm256_extract_epi64(sum_v64, 3);
        // if (len % (BIGMOD_LOOP * 4) < BIGMOD_LOOP)  // No overflow
        // {
        //     for (int j = 0; j < 4; j++)
        //         result += sum_v64_arr[j];
        //     if (result >= BigMod_MagicNumber[0][1])  // avoid overflow
        //         result -= BigMod_MagicNumber[0][1];
        // }
        // else
            for (int j = 0; j < 4; j++) 
            {
                // sum_v64_arr[j] %= BigMod_MagicNumber[0][1];
                if (sum_v64_arr[j] >= BigMod_MagicNumber[0][0])
                    sum_v64_arr[j] -= BigMod_MagicNumber[0][0];
                if (sum_v64_arr[j] >= BigMod_MagicNumber[0][1])
                    sum_v64_arr[j] -= BigMod_MagicNumber[0][1];
                result += sum_v64_arr[j];
                if (result >= BigMod_MagicNumber[0][1])  // avoid overflow
                    result -= BigMod_MagicNumber[0][1];
            }
        for (int i = len - len % 4; i < len; i++)
            result += Vector_Table[i] * ptr[i];

        result %= BIGMOD_MODULI;       // apply modulo operation for final result
        return (uint32_t)result;
    }


    void generate_combination(int num, size_t lower_bound, int total, size_t length, uint64_t remainder1)
    {
        if (lower_bound > length * 32)         // equal is allowed
            return;
        // if (num == total)
        // {
        //     if (remainder1 % moduli[0] == 0 && remainder2 % moduli[1] == 0)
        //         HD[total] += length * 32 - lower_bound + 1;  // any left shift also qualifies
        //     return;
        // }
        for (size_t i = lower_bound; i < length * 32; i++)
        {
            uint32_t positive_error1 = Error_Table[0][i][0];
            uint32_t negative_error1 = Error_Table[0][i][1];
            // Combination[num] = i;
            uint64_t positive_remainder1 = remainder1 + positive_error1;
            
            uint64_t negative_remainder1 = remainder1 + negative_error1;

            if (num + 1 == total)  // early stop
            {
                if (positive_remainder1 % moduli[0] == 0)
                    HD[total] += length * 32 - i;
                if (negative_remainder1 % moduli[0] == 0)
                    HD[total] += length * 32 - i;
                continue;
            }

            generate_combination(num + 1, i + 1, total, length, positive_remainder1);
            generate_combination(num + 1, i + 1, total, length, negative_remainder1);
        }
    }

    void HD_Search(size_t length)
    {
        for (int i = 2; i < BIGMOD_HD_MAX; i++)    // number of errors
        {
            generate_combination(1, 1, i, length, Error_Table[0][0][0]);
            HD[i] = HD[i] * 2;      // dual case: positive and negative
        }
    }

    void scan_32bit()
    {
        auto start_time = std::chrono::steady_clock::now();
        bool* ptr = (bool*)malloc((1ull << 32) * sizeof(bool));
        for (size_t i = 0; i < (1ull << 32); i++)
        {
            ptr[i] = i;
        }
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        printf("Scan 32 bits: %ld ms\n", duration.count());
    }

    void run_perf_test(uint32_t (*func)(uint32_t*, size_t), size_t length, uint32_t* ptr)
    {
        auto start_time = std::chrono::steady_clock::now();
        for (size_t i = 0; i < PERF_COUNT; i++)
            func(ptr + i * length, length); // sequential
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        double volume = length * sizeof(uint32_t) * PERF_COUNT / 1024.0 / 1024.0 / 1024.0;     // GB
        printf("%f GB/s\n", (double)volume / duration.count() * 1000.0);
        return;
    }
    void perf_test(size_t length)
    {
        uint32_t* A = (uint32_t*)malloc(length * sizeof(uint32_t) * PERF_COUNT);
        for (size_t i = 0; i < length * PERF_COUNT; i++)
            A[i] = i;

        printf("BigMod performance test: \n");
        printf("  Bitwise: ");
        run_perf_test(BigMod_Bitwise, length, A);
        printf("  Naive: ");
        run_perf_test(BigMod_Naive, length, A);
        printf("  Table: ");
        run_perf_test(BigMod_Table, length, A); 
        printf("  Table_Iterative: ");
        run_perf_test(BigMod_Table_Iterative, length, A);
        printf("  Table_SIMD: ");
        run_perf_test(BigMod_Table_SIMD, length, A);

        free(A);
    }

    void functional_test(size_t length)
    {
        uint32_t* A = (uint32_t*)malloc(length * sizeof(uint32_t));
        uint32_t* B = (uint32_t*)malloc(length * sizeof(uint32_t));
        for (size_t i = 0; i < length; i++)
        {
            // A[i] = -1;
            A[i] = i + (1 << 30);
            B[i] = 2 * A[i];
        }
        
        // BigMod
        uint32_t ans_Bitwise = BigMod_Bitwise(A, length);
        uint32_t ans_Naive = BigMod_Naive(A, length);
        uint32_t ans_Table = BigMod_Table(A, length);
        uint32_t ans_Table_SIMD = BigMod_Table_SIMD(A, length);
        uint32_t ans_Table_Iterative = BigMod_Table_Iterative(A, length);

        printf("BigMod functional test: ");
        if (ans_Bitwise == ans_Naive && ans_Bitwise == ans_Table && ans_Bitwise == ans_Table_SIMD && ans_Bitwise == ans_Table_Iterative)
            printf("correct. %u %u %u %u %u\n", ans_Bitwise, ans_Naive, ans_Table, ans_Table_SIMD, ans_Table_Iterative);
        else
            printf("incorrect! %u %u %u %u %u\n", ans_Bitwise, ans_Naive, ans_Table, ans_Table_SIMD, ans_Table_Iterative);
    
        free(A);
        free(B);
        return;
    }

    void unit_test()
    {
        printf("\n-----BigMod unit test-----\n");
        printf("BigMod Moduli: %u %u\n", moduli[0], moduli[1]);
        size_t length = max_len;

        // Initialize
        InitMagicNumber();          // Magic number for avoiding modulo operation
        InitLUT(length);            // LUT for table multiplication
        // not used
        InitErrorTable(length);     // Error table for incremental updates and HD search and error correction


        functional_test(length);

        SIMD_test();
        perf_test(length);

        
        // scan_32bit();    // 32-bit DP can help HD search for BigMod
        printf("-----BigMod HD Search-----\n");
        HD_Search(length);
        for (int i = 2; i < BIGMOD_HD_MAX; i++)
            printf("HD[%d]: %u\n", i, HD[i]);


        return;
    }
}