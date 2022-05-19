/**
 * @file mine.c
 *
 * Parallelizes the hash inversion technique used by cryptocurrencies such as
 * bitcoin.
 *
 * Input:    Number of threads, block difficulty, and block contents (string)
 * Output:   Hash inversion solution (nonce) and timing statistics.
 *
 * Compile:  (run make)
 *           When your code is ready for performance testing, you can add the
 *           -O3 flag to enable all compiler optimizations.
 *
 * Run:      ./miner 4 24 'Hello CS 521!!!'
 *
 *   Number of threads: 4
 *     Difficulty Mask: 00000000000000000000000011111111
 *          Block data: [Hello CS 521!!!]
 *
 *   ----------- Starting up miner threads!  -----------
 *
 *   Solution found by thread 3:
 *   Nonce: 10211906
 *   Hash: 0000001209850F7AB3EC055248EE4F1B032D39D0
 *   10221196 hashes in 0.26s (39312292.30 hashes/sec)
 */

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "sha1.h"
#include "logger.h"

unsigned long long total_inversions;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condc = PTHREAD_COND_INITIALIZER;
pthread_cond_t condp = PTHREAD_COND_INITIALIZER;

int buffer = 0;

uint64_t global_nonce = 0;
int rank_found = 0;
char *global_data;
uint32_t global_diff;


struct dataHolder {
    int rank;
    char *data_b;
    uint32_t difficulty_m;
    uint64_t nonce_s;
    // uint64_t nonce_e;
    // uint8_t dige;
};

uint64_t mine(char *data_block, uint32_t difficulty_mask,
        uint64_t nonce_start, uint64_t nonce_end,
        uint8_t digest[SHA1_HASH_SIZE]);

void * producer_thread(){
    return 0;
}

// void * consumer_fast(void * rank){
//     int num = (intptr_t) rank;
//     int nonce_start;
//     if (num == 0){
//         nonce_start = 1;
//     } else {
//         nonce_start = num * 100000;
//     }
//     uint8_t digest[SHA1_HASH_SIZE];

//     pthread_mutex_unlock(&mutex);
//     char *data_block = global_data;
//     uint32_t difficulty_mask = global_diff;
//     pthread_mutex_lock(&mutex);


//     uint64_t nonce = mine(
//             data_block,
//             difficulty_mask,
//             nonce_start, UINT64_MAX,
//             digest);

//     if (nonce != 0){
//         pthread_mutex_unlock(&mutex);
//         global_nonce = nonce;
//         rank_found = num;
//         pthread_mutex_lock(&mutex);
//     }

//     return 0;
    
// }

void * consumer_thread(void *data){
    struct dataHolder *d;
    d = data;
    int rank = d->rank;

    char *data_block = d->data_b;
    uint32_t difficulty_mask = d->difficulty_m;
    uint64_t nonce_start = d->nonce_s;
    // uint64_t nonce_end = d->nonce_e;
    uint8_t digest[SHA1_HASH_SIZE];

    uint64_t nonce = mine(
            data_block,
            difficulty_mask,
            nonce_start, UINT64_MAX,
            digest);

    if (nonce != 0){
        pthread_mutex_unlock(&mutex);
        global_nonce = nonce;
        rank_found = rank;
        pthread_mutex_lock(&mutex);
    }

    free(d);

    return 0;
}

double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void print_binary32(uint32_t num) {
    int i;
    for (i = 31; i >= 0; --i) {
        uint32_t position = (unsigned int) 1 << i;
        printf("%c", ((num & position) == position) ? '1' : '0');
    }
    puts("");
}

uint64_t mine(char *data_block, uint32_t difficulty_mask,
        uint64_t nonce_start, uint64_t nonce_end,
        uint8_t digest[SHA1_HASH_SIZE]) {

    for (uint64_t nonce = nonce_start; nonce < nonce_end; nonce++) {
        /* A 64-bit unsigned number can be up to 20 characters  when printed: */
        size_t buf_sz = sizeof(char) * (strlen(data_block) + 20 + 1);
        char *buf = malloc(buf_sz);

        /* Create a new string by concatenating the block and nonce string.
         * For example, if we have 'Hello World!' and '10', the new string
         * is: 'Hello World!10' */
        snprintf(buf, buf_sz, "%s%lu", data_block, nonce);

        /* Hash the combined string */
        sha1sum(digest, (uint8_t *) buf, strlen(buf));
        free(buf);
        total_inversions++;

        /* Get the first 32 bits of the hash */
        uint32_t hash_front = 0;
        hash_front |= digest[0] << 24;
        hash_front |= digest[1] << 16;
        hash_front |= digest[2] << 8;
        hash_front |= digest[3];

        /* Check to see if we've found a solution to our block */
        if ((hash_front & difficulty_mask) == hash_front) {
            return nonce;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s threads difficulty 'block data (string)'\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]); // TODO
    // num_threads++;
    LOG("Threads: %d\n", num_threads);
    printf("Number of threads: %d\n", num_threads);


    // TODO we have hard coded the difficulty to 20 bits (0x0000FFF). This is a
    // fairly quick computation -- something like 28 will take much longer.  You
    // should allow the user to specify anywhere between 1 and 32 bits of zeros.
    int difficulty = atoi(argv[2]);
    printf("We want this difficulty: %d\n", difficulty);

    uint32_t difficulty_mask = 0b0;
    int num_ones = 32 - difficulty;

    for (int i = 0; i < num_ones; i++){
        difficulty_mask = difficulty_mask | 1 << i;
    }

    difficulty_mask = ~(0);
    difficulty_mask = difficulty_mask >> difficulty;
    global_diff = difficulty_mask;


    // uint32_t difficulty_mask = 0x00000FFF;
    // difficulty_mask = 4095;
    printf("  Difficulty Mask: ");
    print_binary32(difficulty_mask);

    /* We use the input string passed in (argv[3]) as our block data. In a
     * complete bitcoin miner implementation, the block data would be composed
     * of bitcoin transactions. */
    char *bitcoin_block_data = argv[3];
    printf("       Block data: [%s]\n", bitcoin_block_data);

    printf("\n----------- Starting up miner threads!  -----------\n\n");

    double start_time = get_time();

    uint8_t digest[SHA1_HASH_SIZE];

    /* Mine the block. */
    // uint64_t nonce = mine(
    //         bitcoin_block_data,
    //         difficulty_mask,
    //         1, UINT64_MAX,
    //         digest);

    //create threads
    global_data = argv[3];
    pthread_t producer;
    pthread_create(&producer, NULL, producer_thread, NULL);
    pthread_t consumers[num_threads];
    //lock mutex
    pthread_mutex_lock(&mutex);
    uint64_t start = 0;
    for (int i = 0; i < num_threads; i++){
        // void ** temp_arr;
        // temp_arr = malloc(sizeof (void *) * 6);
        // &temp_arr[0] = i;
        // temp_arr[1] = bitcoin_block_data;
        // temp_arr[2] = (unsigned int *) difficulty_mask;
        // temp_arr[3] = (int *) 1;
        // temp_arr[4] = (long unsigned int *) UINT64_MAX;
        // temp_arr[5] = digest;

        struct dataHolder *tempData = malloc(sizeof(struct dataHolder));
        tempData->rank = i;
        tempData->data_b = bitcoin_block_data;
        tempData->difficulty_m = difficulty_mask;
        if (i == 0){
            tempData->nonce_s = 1;
            start = 1;
        } else {
            start += 50000000;
            tempData->nonce_s = start;
        }
        // tempData->nonce_e = UINT64_MAX;
        // tempData->dige = digest;
        pthread_create(&consumers[i], NULL, consumer_thread,
             ((void *) tempData));

        // pthread_create(&consumers[i], NULL, consumer_fast, (void *) (intptr_t) i);
    }

    pthread_join(producer, NULL);
    for (int i = 0; i < num_threads; i++){
        pthread_join(consumers[i], NULL);
    }

    double end_time = get_time();

    if (global_nonce == 0) {
        printf("No solution found!\n");
        return 1;
    }

    /* When printed in hex, a SHA-1 checksum will be 40 characters. */
    char solution_hash[41];
    sha1tostring(solution_hash, digest);

    printf("Solution found by thread %d:\n", rank_found);
    printf("Nonce: %lu\n", global_nonce);
    printf(" Hash: %s\n", solution_hash);

    double total_time = end_time - start_time;
    printf("%llu hashes in %.2fs (%.2f hashes/sec)\n",
            total_inversions, total_time, total_inversions / total_time);

    return 0;
}
