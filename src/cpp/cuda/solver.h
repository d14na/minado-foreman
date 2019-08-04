#ifndef _CUDASOLVER_H_
#define _CUDASOLVER_H_

#include <atomic>
#include <mutex>
#include <string>
#include <vector>


/**
 * (CUDA) Solver
 */
class CUDASolver
{
    /* Definitions */
    public:
        /* Initialize vector. */
        typedef std::vector<uint8_t> bytes_t;

        /* Initialize constants. */
        static const unsigned short ADDRESS_LENGTH = 20u;
        static const unsigned short UINT256_LENGTH = 32u;

    /* Classes */
    public:
        CUDASolver() noexcept;

    /* Variables */
    public:
        static std::atomic<uint32_t> hashes;

    /* Actions */
    public:
        void init();
        void stopSolving();

        bytes_t findSolution();

    /* Setters */
    public:
        void setAddress(std::string const& addr);
        void setChallenge(std::string const& chal);
        void setTarget(std::string const& target);
        void setBlockSize(int size);
        void setThreadSize(int size);

    /* Utilities */
    public:
        static void hexToBytes(std::string const& hex, bytes_t& bytes);
        static std::string bytesToString(bytes_t const& buffer);
        static std::string hexStr(char* data, int len);
        static bool lte(bytes_t const& left, bytes_t const& right);

        bool requiresRestart();

    /* (Private) Variables. */
    private:
        /* Vectors */
        bytes_t m_address;
        bytes_t m_challenge;
        bytes_t m_target, m_target_tmp;
        bytes_t m_buffer, m_buffer_tmp;

        /* Strings */
        std::string s_challenge;
        std::string s_target;

        /* Mutex */
        std::mutex m_buffer_mutex;
        std::mutex m_target_mutex;

        /* Atomic */
        std::atomic<bool> m_buffer_ready;
        std::atomic<bool> m_target_ready;
        std::atomic<bool> m_updated_gpu_inputs;

    /* (Private) Actions */
    private:
        void updateGPULoop();
        void hash(bytes_t const& solution, bytes_t& digest);
};

#endif // !_CUDASOLVER_H_
