#include <chrono>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>

#include "hybrid_ministo.h"

#include "./utilities.cpp"

/**
 * Initialize Globals
 */
std::string mToken;
std::string mChallenge;
std::string mHardwareType;
std::string mMinterAddress;
std::string mTarget;

int shiftStart = 0;
const int MAX_SHIFT_DURATION = 900; // no shift will last longer than 15 mintues

/**
 * Load Command (Text) File
 *
 * NOTE: Located (by default) at `./data/cmd`.
 */
void loadCmdFile()
{
    /* Initialize mining parameters. */
    std::string line;
    std::string lastRequest;
    std::string token;
    std::string challenge;
    std::string target;
    std::string hardwareType;
    std::string minterAddress;

    /* Initialize filename. */
    std::string filename = "./data/cmd";

    /* Initialize command input. */
    std::ifstream cmdFile(filename);

    /* Open file. */
    if (cmdFile.is_open()) {
        while (getline(cmdFile, line)) {
            std::istringstream iss(line);

            /* Validate "stream" parsing. */
            if (!(iss >> lastRequest >> token >> challenge >> target >> hardwareType >> minterAddress)) {
                std::cout << "\nOops! There was an error processing the `cmd` file.\n";

                /* Exit loop. */
                break;
            }

            std::cout << "\n\nMinisto's Command Parameters" << std::endl
                      << "----------------------------" << std::endl
                      << "    Last Requested : " << TimeAgo(lastRequest) << " [ " << filename << " ]" << std::endl
                      << "    Token          : " << token << std::endl
                      << "    Challenge      : " << challenge << std::endl
                      << "    Target         : " << target << std::endl
                      << "    HardwareType   : " << hardwareType << std::endl
                      << "    Minter Address : " << minterAddress << std::endl;

            /* Set mining parameters. */
            mToken = token;
            mChallenge = challenge;
            mTarget = target;
            mHardwareType = hardwareType;
            mMinterAddress = minterAddress;

            break; // FOR DEVELOPMENT PURPOSES ONLY -- LIMITED TO ONE LINE / ONE GPU
        }

        /* Close file. */
        cmdFile.close();
    }
}

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

    /* Validate solution. */
    // NOTE: 32 bytes as a (0x) formatted string
    // FIXME Do we need to perform better validation here??
    if (_hm->solution().size() == 66) {
        /* Get time now. */
        auto completeTime = std::chrono::system_clock::now();

        /* Calculate time difference (in seconds). */
        std::chrono::duration<double> elapsedSeconds = completeTime - nowTime;

        /* Print to console (in machine format). */
        // std::cout << "::NOTIFY:SOLUTION::" << _hm->solution() << ":" << elapsedSeconds.count() << "::" << std::endl;

        /* Retrieve number of hashes. */
        // NOTE: Will also reset the counter.
        int num_hashes = _hm->hashCheck();
        // std::cout << "::NOTIFY:HASHES::" << num_hashes << "::" << std::endl;

        std::cout << CurrentDateAsString(false) << " " << mChallenge << " " << _hm->solution() << " " << num_hashes << " " << elapsedSeconds.count() << std::endl;

        /* Initialize daily activity log file. */
        std::ofstream log("./data/" + CurrentDateAsString() + '-' + mToken + ".log", std::ios_base::app | std::ios_base::out);

        /* Write daily acivity to log. */
        log << CurrentDateAsString(false) << " " << mChallenge << " " << _hm->solution() << " " << num_hashes << " " << elapsedSeconds.count() << std::endl;

        /* Close daily activity log file. */
        log.close();

        /* (Re-)load the command file. */
        loadCmdFile();

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
        /* Load the command file. */
        loadCmdFile();

        /* Handle first parameter (TOKEN). */
        // if (argc > 1) {
        //     /* Validate user input. */
        //     if (std::string(argv[1]) != "")
        //         mToken = argv[1];
        // }

        /* Valdiate token. */
        if (mToken == "") {
            throw std::runtime_error("You MUST provide a 'Token Address' to continue.");
        }

        /* Handle first parameter (CHALLENGE). */
        // if (argc > 2) {
        //     /* Validate user input. */
        //     if (std::string(argv[2]) != "")
        //         mChallenge = argv[2];
        // }

        /* Valdiate challenge. */
        if (mChallenge == "") {
            throw std::runtime_error("You MUST provide a 'Challenge Number' to continue.");
        }

        /* Set challenge. */
        hybrid_ministo->setChallenge(mChallenge);

        /* Handle second parameter (TARGET). */
        // if (argc > 3) {
        //     /* Validate user input. */
        //     if (std::string(argv[3]) != "")
        //         mTarget = argv[3];
        // }

        if (mTarget == "") {
            /* Set default. */
            // NOTE: This is the maximum token mining target of 2^234.
            mTarget = "0x040000000000000000000000000000000000000000000000000000000000";
        }

        /* Set target. */
        hybrid_ministo->setTarget(mTarget);

        /* Handle third parameter (HARDWARE TYPE). */
        // if (argc > 4) {
        //     /* Validate user input. */
        //     if (std::string(argv[4]) != "")
        //         mHardwareType = argv[4];
        // }

        if (mHardwareType == "") {
            /* Set default. */
            mHardwareType = "cpu"; // valid are: cpu, cuda or opencl
        }

        /* Set hardware type. */
        hybrid_ministo->setHardwareType(mHardwareType);

        /* Handle fourth parameter (MINTER ADDRESS). */
        // if (argc > 5) {
        //     /* Validate user input. */
        //     if (std::string(argv[5]) != "")
        //         mMinterAddress = argv[5];
        // }

        /* Validate minter address. */
        if (mMinterAddress == "") {
            /* Set default. */
            mMinterAddress = "0x669008FB464F645a65f8277aB7565e802cDCD5DE";
        }

        /* Set minter address. */
        hybrid_ministo->setMinterAddress(mMinterAddress);

        /* Display all initial settings. */
        std::cout << "\n"
                  << "    Current Mineable Token    [ " << mToken << " ]" << std::endl
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
