#include "EDC.h"

size_t max_len = MAX_LEN;
// unsigned int PERF_COUNT = (1024 * 1024 * 1024) / (max_len * sizeof(uint32_t));
unsigned int PERF_COUNT;

int main(int argc, char** argv)
{
    if (argc > 1)
        max_len = std::atoi(argv[1]);
    PERF_COUNT = (1024 * 1024 * 1024) / (max_len * sizeof(uint32_t));

    printf("Packet length(32 bits): %u\n", (unsigned int)max_len);
    auto start_time = std::chrono::steady_clock::now();

    Linear::unit_test();
    CRC::unit_test();
    BigMod::unit_test();

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    printf("\n-----Finished!-----\nTotal running time: %ld s\n", duration.count() / 1000);
    return 0;
}