#include <chrono>
#include <iostream>

#include "hybrid_ministo.h"

/**
 * Initialize Globals
 */
std::string mChallenge;
std::string mHardwareType;
std::string mMinterAddress;
std::string mTarget;

int shiftStart = 0;
const int MAX_SHIFT_DURATION = 900; // FOR TESTING PURPOSES ONLY

/**
 * Start (Mining)
 */
bool start(HybridMinisto* _hm)
{
    /* Get time now. */
    auto nowTime = std::chrono::system_clock::now();

    /* Convert to seconds (since epoch). */
    std::time_t startTime =
        std::chrono::system_clock::to_time_t(nowTime);

    /* Check shift time. */
    if (shiftStart == 0) {
        shiftStart = startTime;
    } else {
        if (startTime > shiftStart + MAX_SHIFT_DURATION) {
            std::cout << "\n    *** Hey! It looks like your shift is all done. ***\n" << std::endl;

            return 0;
        }
    }

    std::cout << "\nStarting Ministo's Solver @ " << std::ctime(&startTime) << std::endl;

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

    return 0;
}

/**
 * Main (Entry)
 */
int main(int argc, char* argv[])
{
    /* Welcome. */
    std::cout << "\nWelcome to Ministo CLI v19.8.6" << std::endl
              << "----------------------------------------\n" << std::endl;

    /* Initialize instance. */
    HybridMinisto* hybrid_ministo = new HybridMinisto;

    /* Validate miner. */
    if (hybrid_ministo) {
        /* Handle first parameter (TARGET). */
        if (argc > 1) {
            /* Validate user input. */
            if (std::string(argv[1]) != "")
                mTarget = argv[1];
        }

        if (mTarget == "") {
            /* Set default. */
            // NOTE: This is the maximum token mining target of 2^234.
            mTarget = "0x040000000000000000000000000000000000000000000000000000000000";
        }

        /* Set target. */
        hybrid_ministo->setTarget(mTarget);

        /* Handle second parameter (CHALLENGE). */
        if (argc > 2) {
            /* Validate user input. */
            if (std::string(argv[2]) != "")
                mChallenge = argv[2];
        }

        /* Valdiate challenge. */
        if (mChallenge == "") {
            // FOR TESTING PURPOSES ONLY
            mChallenge = "0xc9ee65260340367d976a99c4b77ce5f3a52d70cb5949b5c0066a9d9f7eb340e8";
        }

        /* Set challenge. */
        hybrid_ministo->setChallenge(mChallenge);

        /* Handle third parameter (HARDWARE TYPE). */
        if (argc > 3) {
            /* Validate user input. */
            if (std::string(argv[3]) != "")
                mHardwareType = argv[3];
        }

        if (mHardwareType == "") {
            mHardwareType = "cpu"; // valid are: cpu, cuda or opencl
        }

        /* Set hardware type. */
        hybrid_ministo->setHardwareType(mHardwareType);

        /* Handle fourth parameter (MINTER ADDRESS). */
        if (argc > 4) {
            /* Validate user input. */
            if (std::string(argv[4]) != "")
                mMinterAddress = argv[4];
        }

        /* Validate minter address. */
        if (mMinterAddress == "") {
            /* Set default. */
            mMinterAddress = "0x669008FB464F645a65f8277aB7565e802cDCD5DE";
        }

        /* Set minter address. */
        hybrid_ministo->setMinterAddress(mMinterAddress);

        /* Display all initial settings. */
        std::cout << "\n"
                  << "    Current Difficulty Target [ " << mTarget << " ]" << std::endl
                  << "    Current Mining Challenge  [ " << mChallenge << " ]" << std::endl
                  << "    Current Hardware Type     [ " << mHardwareType << " ]" << std::endl
                  << "    Current Minter's Address  [ " << mMinterAddress << " ]" << std::endl
                  << std::endl;

        /* Start the miner. */
        return start(hybrid_ministo);
    } else {
        // FIXME Let's report this error.
        std::cout << "Oops! `hybrid_ministo` is NOT ready for use." << std::endl;
    }

    /* Exit. */
    return 0;
}
