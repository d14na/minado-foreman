/*******************************************************************************
 * Header file to declare prototypes
 */

#ifndef  _HYBRIDMINISTO_H_
#define  _HYBRIDMINISTO_H_

#include <thread>
#include <string.h>

#include "cpu/solver.h"
// #include "cuda/solver.h"

/**
 * HybridMinisto
 */
class HybridMinisto
{
    /* Classes */
    public:
        HybridMinisto() noexcept;
        ~HybridMinisto();

    /* Actions */
    public:
        void run();
        void stop();

        void setSeed(int const& prngSeed);
        void setChallenge(std::string const& challenge);
        void setTarget(std::string const& target);
        void setMinterAddress(std::string const& minterAddress);

        /* Only used if CUDA mining. */
        void setBlockSize(std::string const& blocksize);
        void setThreadSize(std::string const& threadsize);
        void setHardwareType(std::string const& hardwareType);

        int hashCheck() const;
        std::string solution() const;

    /* (Private) Variables. */
    private:
        /* Values. */
        int m_prngSeed;
        int m_thr_cntr;

        /* Strings */
        std::string m_hardwareType;

        /* Vectors */
        std::vector<CPUSolver> m_solvers;
        std::vector<std::thread> m_threads;

        /* Booleans */
        bool m_bSolutionFound;
        volatile bool m_bExit;

        /* Mutex */
        std::mutex m_solution_mutex;

    /* (CPU) Variables. */
    private:
        CPUSolver::bytes_t m_solution;

    /* (CUDA) Variables. */
    private:
        // CUDASolver cudaSolver;

    /* (Private) Actions */
    private:
        void thr_func(CPUSolver& solver);
        void solutionFound(CPUSolver::bytes_t const& solution);
        /* Set a var in the solver. */
        void set(void (CPUSolver::*fn)(std::string const&), std::string const& p);
};

#endif // ! _HYBRIDMINISTO_H_
