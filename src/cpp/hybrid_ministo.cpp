#include <chrono>
#include <random>
#include <thread>

// NOTE: Allow print out.
#include <iostream>
#include <string.h>

#include "hybrid_ministo.h"


/**
 * Hybrid Ministo (Class)
 */
HybridMinisto::HybridMinisto() noexcept :
    /* Vectors */
    m_solvers(std::thread::hardware_concurrency()),
    m_threads(std::thread::hardware_concurrency()),

    /* Booleans */
    m_bSolutionFound(false),
    m_bExit(false),

    /* (CPU) Vectors */
    m_solution(CPUSolver::UINT256_LENGTH)
{ }


/**
 * Hybrid Ministo (Constructor)
 *
 * Supporting: CPU, Nvidia CUDA and OpenCL devices.
 */
HybridMinisto::~HybridMinisto()
{
    /* Stop solving (if already running). */
    stop();

    /**
     * Wait for run() to terminate.
     *
     * This is not very clean but it's the easiest portable way to
     * exit gracefully if stop() has not been called before the destructor.
     */
    std::this_thread::yield();

    for (auto&& thr : m_threads) {
        if (!thr.joinable())
            std::this_thread::sleep_for(std::chrono::milliseconds(50u));
    }
}

/**
 * Set Hardware Type
 *
 * Available options are: 'cpu' or 'gpu'
 */
void HybridMinisto::setHardwareType(std::string const& hardwareType)
{
    std::cout << "Setting hardware type [ " << hardwareType << " ]\n";

    /* Set hardware type. */
    m_hardwareType = hardwareType;
}

/**
 * Set Challenge (Number)
 */
void HybridMinisto::setChallenge(std::string const& challenge)
{
    if (strcmp(m_hardwareType.c_str(), "cuda") == 0) {
        // cudaSolver.setChallenge(challenge);
    } else {
        set(&CPUSolver::setChallenge, challenge);
    }
}

void HybridMinisto::setTarget(std::string const& target)
{
    if (strcmp(m_hardwareType.c_str(), "cuda") == 0) {
        // cudaSolver.setTarget(target);
    } else {
        set(&CPUSolver::setTarget, target);
    }
}

void HybridMinisto::setMinterAddress(std::string const& minterAddress)
{
    if (strcmp(m_hardwareType.c_str(), "cuda") == 0) {
        // cudaSolver.setAddress(minterAddress);
    } else {
        set(&CPUSolver::setAddress, minterAddress);
    }
}

void HybridMinisto::setBlockSize(std::string const& blocksize)
{
    std::cout << "Setting block size: [ " << blocksize << " ]\n";

    // int i = stoi(blocksize);

    // cudaSolver.setBlockSize(i);
}

void HybridMinisto::setThreadSize(std::string const& threadsize)
{
    std::cout << "Setting thread size: " << threadsize << "\n";

    // int i = stoi(threadsize);

    // cudaSolver.setThreadSize(i);
}

/**
 * RUN
 *
 * This is a the "main" thread of execution.
 */
void HybridMinisto::run()
{
    std::cout << "\n--Starting Ministo.. Hardware type is: " << m_hardwareType.c_str();

    if (strcmp(m_hardwareType.c_str(), "cuda") == 0) {
        /* Initialize solution flag. */
        m_bSolutionFound = false;

        // fill me in ! run  gpu solver
        // CUDASolver::bytes_t solutionBytes(CUDASolver::UINT256_LENGTH);

        // this is core dumping - maybe when you dont have a GPU?
        // cudaSolver.init();

        // solutionBytes = cudaSolver.findSolution();

        std::cout << "\n--GPU returned a solution!! -- \n";

        // This sets m_solution and m_bSolutionFound
        // solutionFound(solutionBytes);

        /* Stop solving. */
        // NOTE: This sets m_bExit true.
        stop();
    } else {
        /* Initialize flags. */
        m_bExit = m_bSolutionFound = false;

        // NOTE: These are the Solver threads.
        for (size_t x = 0; x < m_threads.size(); ++x)
            m_threads[x] = std::thread([&, x] { this->thr_func(this->m_solvers[x]); });

        // ??
        for (auto&& thr: m_threads)
            thr.join();
    }
}

/**
 * Stop
 */
void HybridMinisto::stop()
{
    if (strcmp(m_hardwareType.c_str(), "cuda") == 0) {
        // cudaSolver.stopSolving();
    }

    /* Set exit flag. */
    m_bExit = true;
}

/**
 * Thread Function
 */
void HybridMinisto::thr_func(CPUSolver& solver)
{
    std::random_device r;
    std::mt19937_64 gen(r());
    std::uniform_int_distribution<> dist(0, 0xffffffff);

    CPUSolver::bytes_t solution(CPUSolver::UINT256_LENGTH);

    /* Loop! */
    while (!m_bExit) {
        for (size_t i = 0; i < solution.size(); i += 4) {
            /* Initialize (temp) solution. */
            uint32_t const tmp = dist(gen);

            /* Calculate solution. */
            solution[i]     = static_cast<uint8_t> (tmp & 0x000000ff);
            solution[i + 1] = static_cast<uint8_t>((tmp & 0x0000ff00) >> 8);
            solution[i + 2] = static_cast<uint8_t>((tmp & 0x00ff0000) >> 16);
            solution[i + 3] = static_cast<uint8_t>((tmp & 0xff000000) >> 24);
        }

        /* Try solution. */
        if (solver.trySolution(solution)) {
            /* Report solution. */
            solutionFound(solution);

            break; // Exit while loop.
        }
    }
}

/**
 * Solution Found
 *
 * NOTE: When this function terminates, the "main" thread run() should end
 * and the caller can check the solution().
 */
void HybridMinisto::solutionFound(CPUSolver::bytes_t const& solution)
{
    {
        std::lock_guard<std::mutex> g(m_solution_mutex);

        /* Set solution. */
        m_solution = solution;

        /* Set solution flag. */
        m_bSolutionFound = true;
    }

    /* Stop solving. */
    stop();
}

/**
 * (Return) Solution
 */
std::string HybridMinisto::solution() const
{
    return m_bSolutionFound ? ("0x" + CPUSolver::bytesToString(m_solution)) : std::string();
}

/**
 * Set
 *
 * Edit a variable within each of the solvers.
 */
void HybridMinisto::set(void (CPUSolver::*fn)(std::string const&), std::string const& p)
{
    for (auto&& i : m_solvers)
        (i.*fn)(p);
}
