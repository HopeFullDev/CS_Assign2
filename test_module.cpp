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
  ull dataSection_end = 0x1FFFFFFF;
  ull stackSection_end = 0x1FFFFFFF;
  ull sharedLibSection_end = 0x1FFFFFFF;
  ull HeapSection_end = 0x1FFFFFFF;

  // std::cout << dataSection_end << "  ";
  // Create a random number generator
  auto seed = std::chrono::high_resolution_clock::now()
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
    std::uniform_int_distribution<ull> distr1(random_number_1, textSection_end);
    random_number_2 = distr1(gen);
  } else if (random_number_1 <= dataSection_end &&
             random_number_1 >= DATA_SECTION_START) {
    std::uniform_int_distribution<ull> distr1(random_number_1, dataSection_end);
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
    std::uniform_int_distribution<ull> distr1(random_number_1, HeapSection_end);
    random_number_2 = distr1(gen);
  }

  unsigned long res = (random_number_2 - random_number_1) / 4096;

  // Output the result
  /* std::cout << "T" << task_id << ": ";
   std::cout << "0x" << std::hex << random_number_1;
   std::cout << ": " << std::dec << res << " Bytes\n";
  */
  printf("T%llu:0x%lx:%luB\n", task_id, random_number_1, res);
  return NULL;
}

int main(int argc, char *argv[]) {
  int noOfTrace = 5;
  for (int i = 0; i < noOfTrace; i++) {
    ull temp = i;
    int t = 0;
    pthread_t ptid;
    t = pthread_create(&ptid, NULL, &Genetrate_Traces, &temp);

    // trace.Genetrate_Traces((ull)i);
    //   pthread_exit(NULL);
  }
  pthread_exit(NULL);

  return 0;
}
