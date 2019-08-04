#include <assert.h>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include "solver.h"
// NOTE: We will need this!
#include "sha3.cu"

#include "../utilities.cpp"

// static
std::atomic<uint32_t> CUDASolver::hashes(0u); // statistics only


/**
 * CUDA Solver
 */
CUDASolver::CUDASolver() noexcept :
    m_address(ADDRESS_LENGTH),
    m_challenge(UINT256_LENGTH),
    m_target(UINT256_LENGTH),
    m_target_tmp(UINT256_LENGTH),
    m_buffer(ADDRESS_LENGTH + 2 * UINT256_LENGTH),
    m_buffer_tmp(ADDRESS_LENGTH + 2 * UINT256_LENGTH), //this has something to do with updateBuffer
    m_buffer_ready(false),
    m_target_ready(false),
    m_updated_gpu_inputs(false)
{ }

/**
 * Set Address
 */
void CUDASolver::setAddress(std::string const& addr)
{
    std::cout << "Setting CUDA Address\n";

    /* Validate address length. */
    assert(addr.length() == (ADDRESS_LENGTH * 2 + 2));

    /* Convert from hex to bytes. */
    hexToBytes(addr, m_address);

    /* Set GPU inputs flag. */
    m_updated_gpu_inputs = true;

    /* Update GPU loop. */
    updateGPULoop();
}

/**
 * Set Challenge
 */
void CUDASolver::setChallenge(std::string const& chal)
{
    std::cout << "Setting CUDA Challenge\n";

    /* Validate challenge length. */
    assert(chal.length() == (UINT256_LENGTH * 2 + 2));

    /* Set challenge string. */
    s_challenge = chal;

    /* Convert from hex to bytes. */
    hexToBytes(chal, m_challenge);

    /* Set GPU inputs flag. */
    m_updated_gpu_inputs = true;

    /* Update GPU loop. */
    updateGPULoop();
}

void CUDASolver::setTarget(std::string const& target)
{
    std::cout << "Setting CUDA Target [ " << target << " ]\n";

    assert(target.length() <= (UINT256_LENGTH * 2 + 2));

    std::string const t(static_cast<std::string::size_type>(UINT256_LENGTH * 2 + 2) - target.length(), '0');

    s_target = target;

    /**
     * Double-buffer system, the trySolution() function will be blocked
     * only when a change occurs.
     */
    {
        std::lock_guard<std::mutex> g(m_target_mutex);

        /* Convert from hex to bytes. */
        hexToBytes("0x" + t + target.substr(2), m_target_tmp);
    }

    /* Set target flag. */
    m_target_ready = true;

    /* Set GPU inputs flag. */
    m_updated_gpu_inputs = true;

    /* Update GPU loop. */
    updateGPULoop();
}

/**
 * Set Block Size
 */
void CUDASolver::setBlockSize(int size)
{
    std::cout << "CUDASolver: Setting BlockSize [ " << size << " ]\n";

    setCudaBlockSize(size);
}

/**
 * Set Thread Size
 */
void CUDASolver::setThreadSize(int size)
{
    std::cout << "CUDASolver: Setting Thread Size [ " << size << " ]\n";

    setCudaThreadSize(size);
}

/**
 * Requires Restart
 */
bool CUDASolver::requiresRestart()
{
    return m_updated_gpu_inputs;
}

/**
 * Update GPU Loop
 *
 * NOTE: This will restart the miner if needed.
 */
void CUDASolver::updateGPULoop()
{
    if (m_updated_gpu_inputs
        && m_target_ready
        && m_challenge.size() > 0
        && m_address.size() > 0) {
        /* Set GPU inputs flag. */
        m_updated_gpu_inputs = false;

        printf("Target input:\n");

        /* Validate target length. */
        if (s_target.length() < 66) {
            /* Calculate zero padding (if necessary). */
            std::string zeros = std::string(66 - s_target.length(), '0');

            /* Add zero padding (if necessary). */
            std::string s = "0x" + zeros + s_target.substr(2, s_target.length());

            /* Re-assign target (string). */
            s_target = s;
        }

        /* Initialize target input. */
        unsigned char target_input[64];

        /* Initialize target bytes. */
        bytes_t target_bytes(32);

        /* Convert from hex to bytes. */
        hexToBytes(s_target, target_bytes);

        for (int i = 0; i < 32; i++) {
            target_input[i] = (unsigned char) target_bytes[i];

            printf("%02x", (unsigned char) target_input[i]);
        }

        unsigned char hash_prefix[52];

        std::string clean_challenge = s_challenge;

        bytes_t challenge_bytes(32);

        /* Convert from hex to bytes. */
        hexToBytes(clean_challenge, challenge_bytes);

        for (int i = 0; i < 32; i++) {
            hash_prefix[i] = (unsigned char) challenge_bytes[i];
        }

        for (int i = 0; i < 20; i++) {
            hash_prefix[i+32] = (unsigned char)m_address[i];
        }

        printf("Challenge + Address:\n");

        for (int i = 0; i < 52; i++) {
            printf("%02x", (unsigned char) hash_prefix[i]);
        }

        printf("\n/prefix\n");

        printf("Updating mining inputs..\n");

        update_mining_inputs((const char *)target_input , (const char *)hash_prefix);
    }
}

/**
 * Init
 *
 * NOTE: Call the sha3.cu init func.
 */
void CUDASolver::init()
{
    std::cout << "CUDASolver Initializing.. \n ";

    /* GPU initialization. */
    gpu_init();
}

/**
 * Stop Solving
 */
void CUDASolver::stopSolving( )
{
    std::cout << "CUDA has stopped solving for now.\n ";

    /* Stop solving. */
    stop_solving();
}

/**
 * Find Solution
 */
CUDASolver::bytes_t CUDASolver::findSolution( )
{
    std::cout << "CUDA is trying to find a solution :)\n ";

    /* Set GPU inputs flag. */
    m_updated_gpu_inputs = false;

    /* Create CUDA events. */
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    printf("Target input:\n");

    /* Validate target length. */
    if (s_target.length() < 66) {
        /* Calculate zero padding (if necessary). */
        std::string zeros = std::string(66 - s_target.length(), '0');

        /* Add zero padding (if necessary). */
        std::string s = "0x" + zeros + s_target.substr(2, s_target.length());

        /* Re-assign target (string). */
        s_target = s;
    }

    /* Initialize target input. */
    unsigned char target_input[64];

    /* Initialize target bytes. */
    bytes_t target_bytes(32);

    /* Convert from hex to bytes. */
    hexToBytes(s_target, target_bytes);

    /* Copy target `target_bytes` to `target_input`. */
    for (int i = 0; i < 32; i++) {
  	     target_input[i] = (unsigned char)target_bytes[i];

         // printf("%02x", (unsigned char)target_input[i]);
     }

     /* Initialize hash prefix. */
     unsigned char hash_prefix[52];

     /* Copy challenge (string) to `clean_challenge`. */
     std::string clean_challenge = s_challenge;

     /* Initialize challenge bytes. */
     bytes_t challenge_bytes(32);

     /* Convert from hex to bytes. */
     hexToBytes(clean_challenge, challenge_bytes);

     /* Copy challenge (bytes) to `hash_prefix`. */
     for (int i = 0; i < 32; i++) {
         hash_prefix[i] = (unsigned char)challenge_bytes[i];
     }

     /* Copy address (bytes) to `hash_prefix`. */
     // NOTE: Placed after the challenge.
     for (int i = 0; i < 20; i++) {
         hash_prefix[i + 32] = (unsigned char)m_address[i];
     }

     printf("Challenge + Address:\n");

     for (int i = 0; i < 52; i++) {
         printf("%02x", (unsigned char)hash_prefix[i]);
     }

     printf("\n/Prefix\n");

     unsigned char * s_solution = find_message(
         (const char *)target_input, (const char *)hash_prefix);

     // here

     /* Initialize solution (vector). */
     CUDASolver::bytes_t byte_solution(32);

     /* Copy solution to vector. */
     for (int i = 52; i < 84; i++) {
         byte_solution[i - 52] = (uint8_t)s_solution[i];

         // std::cout << (uint8_t)s_solution[i] << "\n";
     }

     /* Destroy CUDA events. */
     cudaEventDestroy(start);
     cudaEventDestroy(stop);

     /* Return solution. */
     return byte_solution;
}

/**
 * Hex String
 */
std::string CUDASolver::hexStr( char* data, int len)
{
    std::stringstream ss;

    ss << std::hex;

    for (int i = 0; i < len; ++i)
        ss << std::setw(2) << std::setfill('0') << (int)data[i];

    return ss.str();
}

/**
 * Hex To Bytes
 *
 * NOTE: This is a static function.
 */
void CUDASolver::hexToBytes(std::string const& hex, bytes_t& bytes)
{
    assert(hex.length() % 2 == 0);
    assert(bytes.size() == (hex.length() / 2 - 1));

    /* Call Utilities::HexToBytes. */
    HexToBytes(hex.substr(2), &bytes[0]);
}

/**
 * Bytes to String
 *
 * NOTE: This is a static function.
 */
std::string CUDASolver::bytesToString(bytes_t const& buffer)
{
    std::string output;

    output.reserve(buffer.size() * 2 + 1);

    for (unsigned i = 0; i < buffer.size(); ++i)
        output += ascii[buffer[i]];

    return output;
}
