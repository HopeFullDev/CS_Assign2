#include "config.h"
#include <iostream>
#include <random>
// USING ull for all numbers
#define ull unsigned long long

class Test_Module {
public:
  void Genetrate_Traces(ull task_id, ull noOfFramesAlloted,
                        ull firstAllotedFrameNo);
};
void Test_Module ::Genetrate_Traces(ull task_id, ull noOfFramesAlloted,
                                    ull firstAllotedFrameNo) {
  ull min = firstAllotedFrameNo;
  ull max = min + noOfFramesAlloted - 1;

  // Create a random number generator
  std::random_device rd;  // Obtain a random seed from the hardware
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

  // Define a uniform distribution in the range [min, max]
  std::uniform_int_distribution<ull> distr(min, max);

  // Generate and print a random number in the given range
  ull random_number_1 = distr(gen);
  std::uniform_int_distribution<ull> distr1(random_number_1, noOfFramesAlloted);
  ull random_number_2 = distr1(gen);
  ull Base = firstAllotedFrameNo + random_number_1 * PAGE_SIZE;
  std::cout << "T" << task_id << ": ";
  printf(" 0x%llx", Base);
  std::cout << ": "
            << ((random_number_2 - random_number_1) & ((1ULL << 13) - 1))
            << " Bytes\n";
}

int main(int argc, char *argv[]) {
  Test_Module trace;
  int noOfTrace = 10;
  for (int i = 0; i < noOfTrace; i++) {
    trace.Genetrate_Traces((ull)i, (ull)3, (ull)33);
  }

  return 0;
}
