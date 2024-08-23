
class Test_Module {
public:
  void Genetrate_Traces(unsigned long long size, unsigned long long task_id,
                        unsigned long long noOfFramesAlloted,ull firstAllotedFrameNo);
};
Test_Module ::Genetrate_Traces(unsigned long long size,unsigned long long task_id ,unsigned long long noOfFramesAlloted,ull firstAllotedFrameNo )
{
  int min = 1;
    int max = noOfFramesAlloted;

    // Create a random number generator
    std::random_device rd;  // Obtain a random seed from the hardware
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

