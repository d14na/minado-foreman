#ifndef _CUDASHA3_H_
#define _CUDASHA3_H_

/**
 * (CUDA) SHA-3 (aka Keccak-256)
 */
class CUDASha3
{
    /* Classes */
    public:
        CUDASha3() noexcept;

    /* Actions */
    public:
        void gpu_init();
        void setCudaBlockSize(int blocksize);
        void setCudaThreadSize(int threadsize);
        void runBenchmarks();

        char *read_in_messages();

        int gcd(int a, int b);

    /* (Private) Variables. */
    private:
        int clock_speed;
        int number_multi_processors;
        int number_blocks;
        int number_threads;
        int max_threads_per_mp;
        int num_messages;

        /* Constants. */
        const int digest_size = 256;
        const int digest_size_bytes = digest_size / 8;
        const size_t str_length = 7; // change for different sizes

        /* CUDA events. */
        cudaEvent_t start, stop;
};

#endif // !_CUDASHA3_H_
