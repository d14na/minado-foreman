#include <iostream>

#include "hybrid_ministo.h"

int main()
{
    /* Welcome. */
    std::cout << "Ministo CLI" << std::endl;

    /* Initialize instance. */
    // HybridMinisto hybrid_ministo;
    // HybridMinisto* hybrid_ministo = new HybridMinisto();
    HybridMinisto* hybrid_ministo = new HybridMinisto;
    // HybridMinisto* hybrid_ministo = new HybridMinisto();

    if (hybrid_ministo) {
        std::cout << "Starting hybrid_ministo run!" << std::endl;

        hybrid_ministo->setHardwareType("cpu");
        hybrid_ministo->setChallenge("0xc9ee65260340367d976a99c4b77ce5f3a52d70cb5949b5c0066a9d9f7eb340e8");
        hybrid_ministo->setTarget("0x040000000000000000000000000000000000000000000000000000000000");
        hybrid_ministo->setMinterAddress("0x669008FB464F645a65f8277aB7565e802cDCD5DE");

        hybrid_ministo->run(); // blocking call

        // std::cout << "solution: " << hybrid_ministo->solution() << std::endl;
    } else {
        std::cout << "NO hybrid_ministo available!" << std::endl;
    }

    /* Test solution. */
    // std::cout << "Solution -> " << hybrid_ministo.solution() << std::endl;

    return 0;
}
