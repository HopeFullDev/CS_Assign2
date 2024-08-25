#include "config.h"
#include <random>

// USING ull for all numbers
#define ull unsigned long long


class Test_Module {
public:
  void Genetrate_Traces(ull size, ull task_id,
                        ull noOfFramesAlloted,ull firstAllotedFrameNo);
};
void Test_Module ::Genetrate_Traces(ull size,ull task_id ,ull noOfFramesAlloted,ull firstAllotedFrameNo )
{
  int min = (long long)firstAllotedFrameNo;
    int max = noOfFramesAlloted;

    // Create a random number generator
    std::rd;  // Obtain a random seed from the hardware
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    // Define a uniform distribution in the range [min, max]
    std::uniform_int_distribution<> distr(min, max);

    // Generate and print a random number in the given range
    int random_number_1 = distr(gen);
    std::uniform_int_distribution<> distr(random_number_1,noOfFramesAlloted );
  random_number_2=distr(gen);
 ull Base =firstAllotedFrameNo + random_number_1*PAGE_SIZE;
  cout<<"T"<<task_id<<": ";printf(" Value of x %x by printf", Base);cout<<": "<<(random_number_2-random_number_1)*PAGE_SIZE<<" Bytes";
}
}

