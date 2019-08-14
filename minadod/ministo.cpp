#include <chrono>
#include <fstream>
#include <ios>
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
            /* Print to console (in machine format). */
            // std::cout << "::NOTIFY:YOUR_SHIFT_IS_COMPLETE::" << std::endl;

            // return 0;
        }
    }

    std::cout << "\nStarting Ministo's Solver @ " << std::ctime(&startTime) << std::endl;

    /**
     * WARNING!!!
     * THIS IS A BLOCKING CALL
     */
    _hm->run();

    /* Get time now. */
    auto completeTime = std::chrono::system_clock::now();

    /* Calculate time difference (in seconds). */
    std::chrono::duration<double> elapsedSeconds = completeTime - nowTime;

    /* Print to console (in machine format). */
    std::cout << "::NOTIFY:SOLUTION::" << _hm->solution() << ":" << elapsedSeconds.count() << "::" << std::endl;

    /* Retrieve number of hashes. */
    // NOTE: Will also reset the counter.
    int num_hashes = _hm->hashCheck();
    std::cout << "::NOTIFY:HASHES::" << num_hashes << "::" << std::endl;

    /* Initialize daily activity log file. */
    // FIXME: This should be named by date `<token_address>-YYYYMMDD.txt`
    std::ofstream log("./logs.txt", std::ios_base::app | std::ios_base::out);

    /* Write daily acivity to log. */
    log << _hm->solution() << ":" << elapsedSeconds.count() << ":" << num_hashes << std::endl;

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
        /* Handle first parameter (CHALLENGE). */
        if (argc > 1) {
            /* Validate user input. */
            if (std::string(argv[1]) != "")
                mChallenge = argv[1];
        }

        /* Valdiate challenge. */
        if (mChallenge == "") {
            throw std::runtime_error("You MUST provide a 'Challenge Number' to continue.");
        }

        /* Set challenge. */
        hybrid_ministo->setChallenge(mChallenge);

        /* Handle second parameter (TARGET). */
        if (argc > 2) {
            /* Validate user input. */
            if (std::string(argv[2]) != "")
                mTarget = argv[2];
        }

        if (mTarget == "") {
            /* Set default. */
            // NOTE: This is the maximum token mining target of 2^234.
            mTarget = "0x040000000000000000000000000000000000000000000000000000000000";
        }

        /* Set target. */
        hybrid_ministo->setTarget(mTarget);

        /* Handle third parameter (HARDWARE TYPE). */
        if (argc > 3) {
            /* Validate user input. */
            if (std::string(argv[3]) != "")
                mHardwareType = argv[3];
        }

        if (mHardwareType == "") {
            /* Set default. */
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
