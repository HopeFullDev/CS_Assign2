#include "config.h"
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <random>
// USING ull for all numbers
#define ull unsigned long long

void *Genetrate_Traces(void *arg) {
  //  ull task_id) {
  //
  ull task_id = *((ull *)arg);
  ull min = 0x10000000;
  ull max = 0x5FFFFFFF;
  // std::cout<<min <<" "<<max;
  ull textSection_end = 0x1FFFFFFF;
  ull dataSection_end = 0x2FFFFFFF;
  ull stackSection_end = 0x3FFFFFFF;
  ull sharedLibSection_end = 0x4FFFFFFF;
  ull HeapSection_end = 0x5FFFFFFF;

  auto seed = std::chrono::high_resolution_clock::now()
                  .time_since_epoch()
                  .count(); // Obtain a random seed from the hardware
  std::mt19937_64 gen(
      seed); // Standard mersenne_twister_engine seeded with rd()

  std::uniform_int_distribution<ull> enteries(5, 50);

  unsigned long noOfEnteries = enteries(gen);

  while (noOfEnteries > 0) {
    // std::cout << dataSection_end << "  ";
    // Create a random number generator
    seed = std::chrono::high_resolution_clock::now()
               .time_since_epoch()
               .count(); // Obtain a random seed from the hardware
    std::mt19937_64 gen(
        seed); // Standard mersenne_twister_engine seeded with rd()

    // Define a uniform distribution in the range [min, max]
    std::uniform_int_distribution<ull> distr(min, max);

    // Generate and print a random number in the given range
    unsigned long random_number_1 = distr(gen);
    // std::cout<< " "<< random_number_1;

    unsigned long random_number_2 = 0;

    if (random_number_1 <= textSection_end &&
        random_number_1 >= TEXT_SECTION_START) {
      std::uniform_int_distribution<ull> distr1(random_number_1,
                                                textSection_end);
      random_number_2 = distr1(gen);
    } else if (random_number_1 <= dataSection_end &&
               random_number_1 >= DATA_SECTION_START) {
      std::uniform_int_distribution<ull> distr1(random_number_1,
                                                dataSection_end);
      random_number_2 = distr1(gen);
    } else if (random_number_1 <= stackSection_end &&
               random_number_1 >= STACK_SECTION_START) {
      std::uniform_int_distribution<ull> distr1(random_number_1,
                                                stackSection_end);
      random_number_2 = distr1(gen);
    } else if (random_number_1 <= sharedLibSection_end &&
               random_number_1 >= SHARED_LIB_SECTION_START) {
      std::uniform_int_distribution<ull> distr1(random_number_1,
                                                sharedLibSection_end);
      random_number_2 = distr1(gen);
    } else {
      std::uniform_int_distribution<ull> distr1(random_number_1,
                                                HeapSection_end);
      random_number_2 = distr1(gen);
    }

    unsigned long res = (random_number_2 - random_number_1) / 1024;

    // Output the result
    /* std::cout << "T" << task_id << ": ";
     std::cout << "0x" << std::hex << random_number_1;
     std::cout << ": " << std::dec << res << " Bytes\n";
    */
    printf("T%llu:0x%lx:%luKB\n", task_id, random_number_1, res);

    noOfEnteries--;
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  int noOfTrace = 10;
  std::vector<pthread_t> ptid(noOfTrace);

  for (int i = 0; i < noOfTrace; i++) {
    ull *temp = new (ull)(i + 1);
    int t = 0;

    t = pthread_create(&ptid[i], NULL, &Genetrate_Traces, &temp);

    // trace.Genetrate_Traces((ull)i);
    //   pthread_exit(NULL);
  }

  for (int i = 0; i < noOfTrace; i++) {
    pthread_join(ptid[i], NULL);
  }

  pthread_exit(NULL);

  return 0;
}
