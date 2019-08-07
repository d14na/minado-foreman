#include <chrono>
#include <iostream>

#include "hybrid_ministo.h"

int main()
{
    /* Welcome. */
    std::cout << "Ministo CLI" << std::endl;

    /* Initialize instance. */
    HybridMinisto* hybrid_ministo = new HybridMinisto;

    /* Validate miner. */
    if (hybrid_ministo) {
        /* Get time now. */
        auto nowTime = std::chrono::system_clock::now();

        /* Convert to seconds (since epoch). */
        std::time_t startTime =
            std::chrono::system_clock::to_time_t(nowTime);

        std::cout << "Starting hybrid_ministo run @ " << std::ctime(&startTime) << std::endl;

        hybrid_ministo->setHardwareType("cpu");
        hybrid_ministo->setChallenge("0xc9ee65260340367d976a99c4b77ce5f3a52d70cb5949b5c0066a9d9f7eb340e8");
        hybrid_ministo->setTarget("0x040000000000000000000000000000000000000000000000000000000000");
        hybrid_ministo->setMinterAddress("0x669008FB464F645a65f8277aB7565e802cDCD5DE");

        hybrid_ministo->run(); // blocking call

        /* Get time now. */
        nowTime = std::chrono::system_clock::now();

        /* Convert to seconds (since epoch). */
        std::time_t endTime =
            std::chrono::system_clock::to_time_t(nowTime);

        /* Calculate the solve time. */
        int solveTime = (endTime - startTime);

        std::cout << "\nFound a solution: " << hybrid_ministo->solution() << " in " << solveTime << " seconds." << std::endl;
    } else {
        std::cout << "NO hybrid_ministo available!" << std::endl;
    }

    return 0;
}
