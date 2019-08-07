#include <chrono>
#include <iostream>

#include "hybrid_ministo.h"

/**
 * Start (Mining)
 */
void start(HybridMinisto* _hm)
{
    /* Get time now. */
    auto nowTime = std::chrono::system_clock::now();

    /* Convert to seconds (since epoch). */
    std::time_t startTime =
        std::chrono::system_clock::to_time_t(nowTime);

    std::cout << "\nStarting Hybrid Ministo's Solver @ " << std::ctime(&startTime) << std::endl;

    /**
     * WARNING!!!
     * THIS IS A BLOCKING CALL
     */
    _hm->run();

    /* Get time now. */
    nowTime = std::chrono::system_clock::now();

    /* Convert to seconds (since epoch). */
    std::time_t endTime =
        std::chrono::system_clock::to_time_t(nowTime);

    /* Calculate the solve time. */
    int solveTime = (endTime - startTime);

    /* Print to console (in machine format). */
    std::cout << "[[NOTIFY]] [[SOLUTION]] " << _hm->solution() << ":" << solveTime << std::endl;

    /* Validate solution. */
    // NOTE: 32 bytes as a (0x) formatted string
    // FIXME Do we need to perform better validation here??
    if (_hm->solution().size() == 66) {
        /* GO AGAIN!! */
        start(_hm);
    } else {
        std::cout << "Something went wrong. This is the solution we received: " << _hm->solution() << std::endl;
    }
}

/**
 * Main (Entry)
 */
int main()
{
    /* Welcome. */
    std::cout << "\nWelcome to Ministo CLI v19.8.6" << std::endl
              << "----------------------------------------\n" << std::endl;

    /* Initialize instance. */
    HybridMinisto* hybrid_ministo = new HybridMinisto;

    /* Validate miner. */
    if (hybrid_ministo) {
        hybrid_ministo->setHardwareType("cpu");
        hybrid_ministo->setChallenge("0xc9ee65260340367d976a99c4b77ce5f3a52d70cb5949b5c0066a9d9f7eb340e8");
        hybrid_ministo->setTarget("0x040000000000000000000000000000000000000000000000000000000000");
        hybrid_ministo->setMinterAddress("0x669008FB464F645a65f8277aB7565e802cDCD5DE");

        /* Start the miner. */
        start(hybrid_ministo);
    } else {
        // FIXME Let's report this error.
        std::cout << "Oops! `hybrid_ministo` is NOT ready for use." << std::endl;
    }

    /* Exit. */
    return 0;
}
