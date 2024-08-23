// #include <iostream>
// #include <unordered_map>
// #include <vector>
#include "bits/stdc++.h"
#include "config.h"
#include <iostream>
#include <optional>
#include <ostream>
#include <vector>

// USING ull for all numbers
#define ull unsigned long long

// Table for keeping track of how much memory was given to which task
class MM_table {
public:
  //  task id of the task which requested the memory
  ull task_id,
      // size of memory requested by the task
      size,
      // first frame no alloted to the task
      firstAllotedFrameNo,
      // it is possible that the size requested by the task is greater than the
      // page size then multiple frames will be alloted that record is kept here
      noOfFramesAlloted;
};

// class to implement the memory manager it will allot the free frames and keep
// the record of that
class MemoryManager {
public:
  // variable to note the free memory available
  ull freeMemory;
  // variable to note the total no of frames in the physical memory
  ull frameSize;
  // variable to hold the next free frame no
  ull nextFreeFrameNo;
  // variable to track the free frames
  ull noOfFreeFrames;

  MemoryManager() {

    freeMemory = PHYSICAL_MEM_SIZE;

    frameSize = PAGE_SIZE;
    // intialising the free frame no
    nextFreeFrameNo = 0x000000000;
    //    finding the free variable
    noOfFreeFrames = PHYSICAL_MEM_SIZE / PAGE_SIZE;
  }

  // vector to hold the details of which memory is alloted to which task id ,
  // how amany frames alloted , frame no of first frame alloted  and size
  // requested
  std::vector<MM_table> allotedMemoryTable;
  std::optional<ull> assignMemory(ull size, unsigned long long task_id);
};

// The implementation of the memory alloting function for alloting memory to a
// task and updating the record in the vector table and the other member
// variables
std::optional<ull> MemoryManager::assignMemory(unsigned long long size,
                                               unsigned long long task_id) {
  // check if the size requested is larger than available memory
  if (freeMemory > size) {


    ull returnFrameNo = this->nextFreeFrameNo;

    //    adding the mm table entry in the vector
    allotedMemoryTable.push_back({task_id, size, this->nextFreeFrameNo,
                                  static_cast<ull>(ceil(size / PAGE_SIZE))});
    ull noOfFramesAlloted = static_cast<ull>(ceil(size / PAGE_SIZE));
    // updating the free Memory available
    this->freeMemory -= (noOfFramesAlloted * PAGE_SIZE);
    // now updating the nextFreeFrameNo
    MM_table &lastElement = allotedMemoryTable.back();

    this->nextFreeFrameNo =
        lastElement.firstAllotedFrameNo +
        lastElement.noOfFramesAlloted /*ceil(size/PAGE_SIZE)+1*/;
    return returnFrameNo;
  }
  // in case if condidtion is alse then reaturn an error in the form of sending
  // max element of unsigned long long we have to handle this case while calling
  // this function
  return std::nullopt;
}

// a universal instance of memory manager for keeping it seperate from the tasks
static MemoryManager mmInstance;



class Test_Module
{
public:
Genetrate_Traces(unsigned long long size,unsigned long long task_id, unsigned long long noOfFramesAlloted );
};
Test_Module ::Genetrate_Traces(unsigned long long size,unsigned long long task_id ,unsigned long long noOfFramesAlloted )
{
  int min = 1;
    int max = noOfFramesAlloted;

    // Create a random number generator
    std::random_device rd;  // Obtain a random seed from the hardware
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    // Define a uniform distribution in the range [min, max]
    std::uniform_int_distribution<> distr(min, max);

    // Generate and print a random number in the given range
    int random_number = distr(gen);

  if()
}
// this class will define the three implementation of page tables and the
// associated variables to track the allocation of memory
class Task {
public:
  //  each task will have some id attached to it that is stored in this variable
  ull task_id;
  //  the size of page table
  ull pageTableSize;
  // no of pages in the page table these page no will be mapped to frame no in
  // physical memory for implemenation type A and B
  ull noOfPages;
  //  Variables to keep track of page hits in different implementations
  ull pageHitImplementationA, pageHitImplementationB, pageHitImplementationC;
  //  Variables to keep track of page miss/faults in different implementations
  ull pageFaultImplementationA, pageFaultImplementationB,
      pageFaultImplementationC;

  // this is the first implementation of page table as key value in a hash map
  std::unordered_map<ull, ull> PageTableImplementationA;

  // the second implementation of page table as single level page table
  // we create the page table as a vector size = Page size
  std::vector<ull> PageTableImplementationB;

  // this function will request the memory which is frame no from the memory
  // manager , and in return it will get the first alloted frame no in case of
  // requested size is larger than frame size we will have to handle that in
  // this as well , also is will also check of the requested virtual address
  // already exists in the page table
  void requestMemory(ull size, ull virtual_address);

  Task() {
    pageTableSize = PAGE_SIZE;
    noOfPages = VIRTUAL_MEM_SIZE / pageTableSize;
    PageTableImplementationB.resize((ull)pageTableSize);
    pageHitImplementationA = 0;
    pageHitImplementationB = 0;
    pageHitImplementationC = 0;
    pageFaultImplementationA = 0;
    pageFaultImplementationB = 0;
    pageFaultImplementationC = 0;
  }
};
void Task::requestMemory(ull size, ull virtual_address) {
  // get the page number for the virtual address where
  ull pageNoOfVirtualAddress = virtual_address / pageTableSize;
  // first lets deal wiht implemenattion A
  //  Check if page aldready exists
  if (PageTableImplementationA.find(pageNoOfVirtualAddress) !=
      PageTableImplementationA.end()) {
    ++pageHitImplementationA;
    return;

  }
  // for implemantation A the page does not exist. so request the page
  else {
    ++pageFaultImplementationA;
    auto frameNo = mmInstance.assignMemory(size, this->task_id);

    if (!frameNo.has_value()) {
      // in case the requested size is larger than available memory
      std::cout << "Task Id: " << this->task_id << " requested " << size
                << " in bytes but available memory is only "
                << mmInstance.freeMemory << " in bytes" << std::endl;
      return;
    } else {
      // here we know that we have been alloted some frames
      ull initialFrameNo = frameNo.value();
      // now in case the requested memory is larger than single page we have to
      // handle that here
      ull noOfFramesAlloted = (size + PAGE_SIZE - 1) / PAGE_SIZE;

      for (ull i = 0; i < noOfFramesAlloted; i++) {
        PageTableImplementationA[pageNoOfVirtualAddress + i] =
            initialFrameNo + i;
      }
    }
  }
  return;
}

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

