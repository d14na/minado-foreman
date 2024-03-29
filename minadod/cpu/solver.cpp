#include <assert.h>

#include "solver.h"
#include "sha3.h"

#include "../utilities.cpp"

// static
std::atomic<uint32_t> CPUSolver::hashes(0u); // statistics only


/**
 * CPU Solver
 */
CPUSolver::CPUSolver() noexcept :
    m_address(ADDRESS_LENGTH),
    m_challenge(UINT256_LENGTH),
    m_target(UINT256_LENGTH),
    m_target_tmp(UINT256_LENGTH),
    m_buffer(ADDRESS_LENGTH + 2 * UINT256_LENGTH),
    m_buffer_tmp(ADDRESS_LENGTH + 2 * UINT256_LENGTH),
    m_buffer_ready(false),
    m_target_ready(false)
{ }

/**
 * Set Address
 */
void CPUSolver::setAddress(std::string const& addr)
{
    /* Validate address. */
    // NOTE: Includes `0x` prefix.
    assert(addr.length() == (ADDRESS_LENGTH * 2 + 2));

    /* Convert from hex to bytes. */
    hexToBytes(addr, m_address);

    /* Update buffer. */
    updateBuffer();
}

/**
 * Set Challenge
 */
void CPUSolver::setChallenge(std::string const& chal)
{
    /* Validate challenge. */
    assert(chal.length() == (UINT256_LENGTH * 2 + 2));

    /* Convert from hex to bytes. */
    hexToBytes(chal, m_challenge);

    /* Update buffer. */
    updateBuffer();
}

/**
 * Set Target
 */
void CPUSolver::setTarget(std::string const& target)
{
    /* Valiate target. */
    assert(target.length() <= (UINT256_LENGTH * 2 + 2));

    /* Zero padding. */
    std::string const zero_pad(
        static_cast<std::string::size_type>(UINT256_LENGTH * 2 + 2) - target.length(),
        '0');

    /**
     * Double-buffer system, the trySolution() function will be blocked
     * only when a change occurs.
     */
    {
        /* Initialize (guarded) mutex. */
        std::lock_guard<std::mutex> g(m_target_mutex);

        /* Convert from hex to bytes. */
        hexToBytes("0x" + zero_pad + target.substr(2), m_target_tmp);
    }

    /* Set target flag. */
    m_target_ready = true;
}

/**
 * Update Buffer
 *
 * Buffer order:
 *     1 - Challenge
 *     2 - Minter (ETH) Address (will typically belong to a pool)
 *     3 - Solution / Nonce
 */
void CPUSolver::updateBuffer()
{
    /**
     * The idea is to have a double-buffer system in order not to try
     * to acquire a lock on each hash() loop.
     */
    {
        /* Initialize (guarded) mutex. */
        std::lock_guard<std::mutex> g(m_buffer_mutex);

        /* Copy challenge into buffer. */
        std::copy(m_challenge.cbegin(), m_challenge.cend(), m_buffer_tmp.begin());

        /* Copy address into buffer (after challenge). */
        std::copy(m_address.cbegin(), m_address.cend(), m_buffer_tmp.begin() + m_challenge.size());
    }

    /* Set buffer flag. */
    m_buffer_ready = true;
}

/**
 * (Keccak) Hash
 */
void CPUSolver::hash(bytes_t const& solution, bytes_t& digest)
{
    /* Validate buffer. */
    if (m_buffer_ready) {
        /* Initialize (guarded) mutex. */
        std::lock_guard<std::mutex> g(m_buffer_mutex);

        /* Swap buffer. */
        m_buffer.swap(m_buffer_tmp);

        /* Set buffer flag. */
        m_buffer_ready = false;
    }

    /* Copy solution to buffer (after challenge and address). */
    std::copy(
        solution.cbegin(),
        solution.cend(),
        m_buffer.begin() + m_challenge.size() + m_address.size());

    /* Perform (keccak) hash. */
    keccak_256(&digest[0], digest.size(), &m_buffer[0], m_buffer.size());
}

/**
 * Try Solution
 */
bool CPUSolver::trySolution(bytes_t const& solution)
{
    /* Initialize digest. */
    bytes_t digest(UINT256_LENGTH);

    /* Request (keccak) hash. */
    hash(solution, digest);

    /* Validate target. */
    if (m_target_ready) {
        /* Initialize (guarded) mutex. */
        std::lock_guard<std::mutex> g(m_target_mutex);

        /* Swap buffer. */
        m_target.swap(m_target_tmp);

        /* Set target flag. */
        m_target_ready = false;
    }

    /* Increment hash counter. */
    ++hashes;

    /* Return less-than-or-equal test. */
    return lte(digest, m_target);
}

/**
 * Hex To Bytes
 *
 * NOTE: This is a static function.
 */
void CPUSolver::hexToBytes(std::string const& hex, bytes_t& bytes)
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
std::string CPUSolver::bytesToString(bytes_t const& buffer)
{
    /* Initialize output. */
    std::string output;

    output.reserve(buffer.size() * 2 + 1);

    for (unsigned i = 0; i < buffer.size(); ++i)
        output += ascii[buffer[i]];

    /* Return output. */
    return output;
}

/**
 * Less-Than-Or-Equal
 *
 * Compare byte-by-byte from left to right.
 *
 * NOTE: This is a static function.
 */
bool CPUSolver::lte(bytes_t const& left, bytes_t const& right)
{
    /* Validate inputs. */
    assert(left.size() == right.size());

    for (unsigned i = 0; i < left.size(); ++i) {
        if (left[i] == right[i])
            continue;

        if (left[i] > right[i])
            return false;

        return true;
    }

    return true;
}
