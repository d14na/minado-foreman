/*******************************************************************************
 * Author: Brian Bowden
 * Date: 5/12/14
 *
 * This is the parallel version of SHA-3.
 *
 * Author: Mikers
 * Date: March 4, 2018
 * Purpose: For 0xbitcoin development.
 * (based off of https://github.com/Dunhili/SHA3-gpu-brute-force-cracker/blob/master/sha3.cu)
 */

/* TODO: These magic numbers need tuning. */
#define LOOP_IN_GPU_OPTIMIZATION 1000
#define HARDCODED_BLOCKSIZE 256
#define HARDCODED_THREADSIZE 256

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <time.h>
#include <curand.h>
#include <assert.h>
#include <curand_kernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "sha3.h"

// void gpu_init();
// void runBenchmarks();

char *read_in_messages();

int gcd(int a, int b);

// updated message the gpu_init() function
int clock_speed;
int number_multi_processors;
int number_blocks;
int number_threads;
int max_threads_per_mp;
int h_done[1] = {0};

unsigned long long cnt = 0;

int num_messages;

const int digest_size = 256;
const int digest_size_bytes = digest_size / 8;
const size_t str_length = 7;	//change for different sizes

int gpu_blocksize = HARDCODED_BLOCKSIZE;
int gpu_threadsize = HARDCODED_THREADSIZE;

// cudaEvent_t start, stop;

#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))

__device__ const char *chars =
    " !\"#$%&\'()*+'-./0123456789:;<=>?@ABCDEFGHIJKLMOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

__device__ const uint64_t RC[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

__device__ const int r[24] = {
    1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
    27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
};

__device__ const int piln[24] = {
    10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
};

/**
 * Compare Hash
 *
 * NOTE: Device ONLY.
 */
__device__ int compare_hash(
    unsigned char *target,
    unsigned char *hash,
    int length)
{
    /* Initialize index. */
	int i = 0;

    /* Loop through bytes, until non-matching bytes are found. */
	for (i = 0; i < length; i++) {
		if (hash[i] != target[i])
            break;
	}

    /* Compare the bytes. */
    // NOTE: Must be LESS THAN OR EQUAL to the target.
    return (unsigned char)(hash[i]) <= (unsigned char)(target[i]);
}

/**
 * Keccak-256
 *
 * NOTE: Device ONLY.
 */
__device__ void keccak256(uint64_t state[25])
{
    uint64_t temp, C[5];
	int j;

    for (int i = 0; i < 24; i++) {
    // Theta
		C[0] = state[0] ^ state[5] ^ state[10] ^ state[15] ^ state[20];
		C[1] = state[1] ^ state[6] ^ state[11] ^ state[16] ^ state[21];
		C[2] = state[2] ^ state[7] ^ state[12] ^ state[17] ^ state[22];
		C[3] = state[3] ^ state[8] ^ state[13] ^ state[18] ^ state[23];
		C[4] = state[4] ^ state[9] ^ state[14] ^ state[19] ^ state[24];

		temp = C[4] ^ ROTL64(C[1], 1);
		state[0] ^= temp;
		state[5] ^= temp;
		state[10] ^= temp;
		state[15] ^= temp;
		state[20] ^= temp;

		temp = C[0] ^ ROTL64(C[2], 1);
		state[1] ^= temp;
		state[6] ^= temp;
		state[11] ^= temp;
		state[16] ^= temp;
		state[21] ^= temp;

		temp = C[1] ^ ROTL64(C[3], 1);
		state[2] ^= temp;
		state[7] ^= temp;
		state[12] ^= temp;
		state[17] ^= temp;
		state[22] ^= temp;

		temp = C[2] ^ ROTL64(C[4], 1);
		state[3] ^= temp;
		state[8] ^= temp;
		state[13] ^= temp;
		state[18] ^= temp;
		state[23] ^= temp;

		temp = C[3] ^ ROTL64(C[0], 1);
		state[4] ^= temp;
		state[9] ^= temp;
		state[14] ^= temp;
		state[19] ^= temp;
		state[24] ^= temp;

    // Rho Pi
		temp = state[1];
		j = piln[0];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[0]);
		temp = C[0];

		j = piln[1];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[1]);
		temp = C[0];

		j = piln[2];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[2]);
		temp = C[0];

		j = piln[3];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[3]);
		temp = C[0];

		j = piln[4];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[4]);
		temp = C[0];

		j = piln[5];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[5]);
		temp = C[0];

		j = piln[6];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[6]);
		temp = C[0];

		j = piln[7];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[7]);
		temp = C[0];

		j = piln[8];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[8]);
		temp = C[0];

		j = piln[9];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[9]);
		temp = C[0];

		j = piln[10];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[10]);
		temp = C[0];

		j = piln[11];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[11]);
		temp = C[0];

		j = piln[12];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[12]);
		temp = C[0];

		j = piln[13];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[13]);
		temp = C[0];

		j = piln[14];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[14]);
		temp = C[0];

		j = piln[15];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[15]);
		temp = C[0];

		j = piln[16];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[16]);
		temp = C[0];

		j = piln[17];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[17]);
		temp = C[0];

		j = piln[18];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[18]);
		temp = C[0];

		j = piln[19];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[19]);
		temp = C[0];

		j = piln[20];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[20]);
		temp = C[0];

		j = piln[21];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[21]);
		temp = C[0];

		j = piln[22];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[22]);
		temp = C[0];

		j = piln[23];
		C[0] = state[j];
		state[j] = ROTL64(temp, r[23]);
		temp = C[0];

    // Chi
		C[0] = state[0];
		C[1] = state[1];
		C[2] = state[2];
		C[3] = state[3];
		C[4] = state[4];

		state[0] ^= (~C[1]) & C[2];
		state[1] ^= (~C[2]) & C[3];
		state[2] ^= (~C[3]) & C[4];
		state[3] ^= (~C[4]) & C[0];
		state[4] ^= (~C[0]) & C[1];

		C[0] = state[5];
		C[1] = state[6];
		C[2] = state[7];
		C[3] = state[8];
		C[4] = state[9];

		state[5] ^= (~C[1]) & C[2];
		state[6] ^= (~C[2]) & C[3];
		state[7] ^= (~C[3]) & C[4];
		state[8] ^= (~C[4]) & C[0];
		state[9] ^= (~C[0]) & C[1];

		C[0] = state[10];
		C[1] = state[11];
		C[2] = state[12];
		C[3] = state[13];
		C[4] = state[14];

		state[10] ^= (~C[1]) & C[2];
		state[11] ^= (~C[2]) & C[3];
		state[12] ^= (~C[3]) & C[4];
		state[13] ^= (~C[4]) & C[0];
		state[14] ^= (~C[0]) & C[1];

		C[0] = state[15];
		C[1] = state[16];
		C[2] = state[17];
		C[3] = state[18];
		C[4] = state[19];

		state[15] ^= (~C[1]) & C[2];
		state[16] ^= (~C[2]) & C[3];
		state[17] ^= (~C[3]) & C[4];
		state[18] ^= (~C[4]) & C[0];
		state[19] ^= (~C[0]) & C[1];

		C[0] = state[20];
		C[1] = state[21];
		C[2] = state[22];
		C[3] = state[23];
		C[4] = state[24];

		state[20] ^= (~C[1]) & C[2];
		state[21] ^= (~C[2]) & C[3];
		state[22] ^= (~C[3]) & C[4];
		state[23] ^= (~C[4]) & C[0];
		state[24] ^= (~C[0]) & C[1];

    // Iota
        state[0] ^= RC[i];
    }
}

/**
 * Keccak
 *
 * NOTE: Device ONLY.
 */
__device__ void keccak(
    const char *message,
    int message_len,
    unsigned char *digest,
    int digest_len)
{
    uint64_t state[25];
    uint8_t temp[144];
    int rsize = 136;
    int rsize_byte = 17;

    memset(state, 0, sizeof(state));

    for ( ; message_len >= rsize; message_len -= rsize, message += rsize) {
        for (int i = 0; i < rsize_byte; i++) {
            state[i] ^= ((uint64_t *) message)[i];
		}

        keccak256(state);
    }

    // last block and padding
    memcpy(temp, message, message_len);

    temp[message_len++] = 1;

    memset(temp + message_len, 0, rsize - message_len);

    temp[rsize - 1] |= 0x80;

    for (int i = 0; i < rsize_byte; i++) {
        state[i] ^= ((uint64_t *) temp)[i];
	}

    keccak256(state);

    memcpy(digest, state, digest_len);
}

/**
 * GPU Mine
 *
 * NOTE: This function uses the `__global__` declaration; and therefore
 * will be executed as a "kernel" within the GPU cores.
 *
 * NOTE: Hash length is 256 bits.
 */
__global__ void gpu_mine(
    char * working_memory_hash,
    char * working_memory_nonce,
    unsigned char *challenge_hash,
    char * device_solution,
    int *done,
    const unsigned char * hash_prefix,
    int now,
    int cnt)
{
    /* Create a new thread id. */
    int tid = threadIdx.x + (blockIdx.x * blockDim.x);

    char * message = &working_memory_nonce[84 * tid];

    char * hash = &working_memory_hash[32 * tid];

    /* Initialize string length. */
    // NOTE: This is challenge[32] + address[20] + nonce[32].
    int str_len = 84;

    curandState_t state;
    /* we have to initialize the state */

    curand_init(now, tid, cnt, &state);

    int len = 0;

    for (len = 0; len < 52; len++) {
		message[len] = hash_prefix[len];
	}

    for (int i = 0; i < LOOP_IN_GPU_OPTIMIZATION; i++) {
        /* Generate a new nonce. */
        for (len = 0; len < 32; len++) {
            /* Generate random byte. */
            char r = (char)curand(&state) % 256;

            /* Add byte to message. */
            message[52 + len] = r;
        }

        /* Initialize digest length. */
        const int digest_len = 32;

        /* Initialize digest. */
        unsigned char digest[digest_len];

        /* Perform keccak (on the message). */
        keccak(&message[0], str_len, &digest[0], digest_len);

        /* Validate our solution (is less than target). */
        if (compare_hash(&challenge_hash[0], &digest[0], digest_len)) {
            if (done[0] != 1) {
                /* Set flag. */
                done[0] = 1;

                /* Copy the solution. */
                memcpy(device_solution, message, str_len);
            }

            /* Exit. */
            return;
        }
    }
}

/**
 * Stop Solving
 */
void stop_solving()
{
    /* Set flag. */
    h_done[0] = 1 ;
}

/**
 * Set CUDA Block Size
 */
void setCudaBlockSize(int blocksize)
{
   gpu_blocksize = blocksize;
}

/**
 * Set CUDA Thread Size
 */
void setCudaThreadSize(int threadsize)
{
   gpu_threadsize = threadsize;
}

/**
 * GPU Initialization
 */
void gpu_init()
{
    cudaDeviceProp device_prop;

    int device_count, block_size;

    cudaGetDeviceCount(&device_count);

    if (cudaGetDeviceProperties(&device_prop, 0) != cudaSuccess) {
        printf("Problem getting properties for device, exiting...\n");

        exit(EXIT_FAILURE);
    }

    number_threads = device_prop.maxThreadsPerBlock;

    number_multi_processors = device_prop.multiProcessorCount;

    max_threads_per_mp = device_prop.maxThreadsPerMultiProcessor;

    number_threads = gpu_threadsize;

    number_blocks = gpu_blocksize;

    clock_speed = (int) (device_prop.memoryClockRate * 1000 * 1000);    // convert from GHz to hertz
}

/**
 * Greatest Common Denominator
 */
int gcd(int a, int b) {
    return (a == 0) ? b : gcd(b % a, a);
}

/**
 * Hash Count
 */
int getHashCount() {
    return cnt;
}

/**
 * Reset Hash Count
 */
void resetHashCount() {
    cnt = 0;
}

/**
 * Update Mining Inputs
 *
 * Initializes hashing variables on the device; then copies data
 * from host memory to the device memory.
 */
void update_mining_inputs(
    const char * challenge_target,
    const char * hash_prefix) // can accept challenge
{
    /* Initialize device variables. */
    int *d_done;
    unsigned char *d_hash;
    char *device_solution;
    unsigned char * d_challenge_hash;
    unsigned char * d_hash_prefix;

    /* Allocate memory on device. */
    cudaMalloc((void**) &d_done, sizeof(int));
    cudaMalloc((void**) &device_solution, 84); // solution
    cudaMalloc((void**) &d_challenge_hash, 32);
    cudaMalloc((void**) &d_hash_prefix, 52);

    /* Copy host memory (data) to the device memory. */
    cudaMemcpy(d_done, h_done, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_challenge_hash, challenge_target, 32, cudaMemcpyHostToDevice);
    cudaMemcpy(d_hash_prefix, hash_prefix, 52, cudaMemcpyHostToDevice);
}

/**
 * Find Message
 */
unsigned char * find_message(
    const char * challenge_target,
    const char * hash_prefix) // can accept challenge
{
    /* Initialize done. */
    h_done[0] = 0;

    /* Initialize device variables. */
    int *d_done;
    char *device_solution;
    unsigned char * d_challenge_hash;
    unsigned char * d_hash_prefix;

    /* Allocate memory on device. */
    cudaMalloc((void**) &d_done, sizeof(int));
    cudaMalloc((void**) &device_solution, 84); // solution
    cudaMalloc((void**) &d_challenge_hash, 32);
    cudaMalloc((void**) &d_hash_prefix, 52);

    /* Copy host memory (data) to the device memory. */
    cudaMemcpy(d_done, h_done, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_challenge_hash, challenge_target, 32, cudaMemcpyHostToDevice);
    cudaMemcpy(d_hash_prefix, hash_prefix, 52, cudaMemcpyHostToDevice);

    /* Set CUDA thread limit. */
    cudaThreadSetLimit(cudaLimitMallocHeapSize,
        2 * ((84 * number_blocks * number_threads) + (32 * number_blocks * number_threads)));

    /* Initialze (count of) workers. */
    unsigned int workers = number_blocks * number_threads;

    /* Initialize (managed) memory hash. */
    char * working_memory_hash;
    cudaMallocManaged(&working_memory_hash, workers * 32);

    /* Initialize (managed) memory nonce. */
    char * working_memory_nonce;
    cudaMallocManaged(&working_memory_nonce, workers * 84);

    /* Initialize (time) now. */
    int now = (int)time(0);

    /* Reset hash count. */
    // cnt = 0;
    resetHashCount();

    printf("USING NB %d NT %d.\n", number_blocks, number_threads);

    while (!h_done[0]) {
        /* Call the kernel (with launch parameters). */
        gpu_mine<<<number_blocks, number_threads>>>(
            working_memory_hash,
            working_memory_nonce,
            d_challenge_hash,
            device_solution,
            d_done,
            d_hash_prefix,
            now,
            cnt
        );

        /* Perform device synchronization. */
        cudaError_t cudaerr = cudaDeviceSynchronize();

        /* Validate synchronization success. */
        if (cudaerr != cudaSuccess) {
            /* Set done flag. */
            h_done[0] = 1;

            printf("Kernel launch failed with error \"%s\".\n", cudaGetErrorString(cudaerr));

            /* Exit application. */
            exit(EXIT_FAILURE);
        }

        /* Increment hash count. */
        cnt += number_threads * number_blocks * LOOP_IN_GPU_OPTIMIZATION;

        fprintf(stderr, "Total Hashes: %u\n", cnt);

        /* Copy device `done` value back to host memory. */
        cudaMemcpy(h_done, d_done, sizeof(int), cudaMemcpyDeviceToHost);
    }

    /* Initialize (host) message. */
    unsigned char * h_message = (unsigned char*)malloc(84);

    /* Copy device `solution` value back to host memory. */
    cudaMemcpy(h_message, device_solution, 84, cudaMemcpyDeviceToHost);

    /* Initialize file pointer. */
    // FIXME: Why are we writing this file??
    FILE * fp;

    /* Open file. */
    fp = fopen ("out.binary", "wb") ;

    /* Write message. */
    fwrite(h_message, 84, 1, fp);

    /* Close file. */
    fclose(fp);

    fprintf(stderr, "Total hashes: %u\n", cnt);

    /* Free memory. */
    cudaFree(d_done);
    cudaFree(device_solution);
    cudaFree(d_challenge_hash);
    cudaFree(d_hash_prefix);
    cudaFree(working_memory_hash);
    cudaFree(working_memory_nonce);

    /* Return message. */
    return h_message;
}

/**
 * Initialization
 *
 * Main method, initializes the global variables, calls the kernels,
 * and prints the results.
 */
int init(int argc, char **argv)
{
    char * hash_prefix_filename = argv[1];
	char * challenge = argv[2]; // challenge is the target
	char  hash_prefix[53];

    FILE *f = fopen(hash_prefix_filename, "r");

    fread(&hash_prefix, 52, 1, f);

	hash_prefix[52] = '\0';

    srand(time(0));

	char challenge_target[32];

    FILE *fc = fopen(challenge, "r");

    fread(&challenge_target, 32, 1, fc);

    /* Initialize GPU. */
	gpu_init();

    /* Find a message (solution). */
	find_message(challenge_target, hash_prefix);

    /* Exit application. */
	return EXIT_SUCCESS;
}
