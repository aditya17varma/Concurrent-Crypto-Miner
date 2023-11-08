# Parallel Cryptocurrency Miner
Parallel computation of the hash inversion technique used by cryptocurrencies such as bitcoin.


In Bitcoin, hash inversions are the computationally expensive problem being solved by the computer. Given a block, the algorithm tries to find a nonce (number only used once) that when combined with the block data produces a hash code with a set amount of leading zeros. The more leading zeros requested, the harder the problem is to solve. It’s kind of like rolling dice; rolling any number from 1 through 3 is much easier than rolling a 6. 

This project uses threads and parallel computation to find the solution to the block.

## Features
- Choose the number of threads to use.
- Choose the difficulty mask.
- Choose the string to hash.

The main thread will product tasks, and each worker thread performs the hash inversions. If the user specifies `4` for the thread count, this means you'll have five total threads (main thread + 4 workers).


## Run
1. Run make.
2. ./miner <numThreads> <difficultyMask> <string>

Running the program with 4 threads, a difficulty mask of 24, and block data 'Hello test 15!!!':

```
./miner 4 24 'Hello test 15!!!'

Number of threads: 4
We want this difficulty: 24
  Difficulty Mask: 00000000000000000000000011111111
       Block data: [Hello test 15!!!]

----------- Starting up miner threads!  -----------

miner.c:79:consumer_thread(): rank:0
miner.c:79:consumer_thread(): rank:1
miner.c:79:consumer_thread(): rank:2
miner.c:79:consumer_thread(): rank:3
Solution found by thread: 2
Nonce: 5566227
Hash: 686213DEFC7F00001047B4FA377F000000000000
20775947 hashes in 10.71s (1940356.92 hashes/sec)
```



