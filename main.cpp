// #include <iostream>
// #include <unordered_map>
// #include <vector>
#include "bits/stdc++.h"
#include "config.h"

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

// this class will define the three implementation of page tables and the
// associated variables to track the allocation of memory
class Task {
public:
  //  each task will have some id attached to it that is stored in this variable
  ull task_id;
  //  the size of page table
  ull pageTableSize;
  // no of pages in the page table these page no will be mapped to frame no in
  // physical memory
  ull noOfPages;

  // this is the first implementation of page table as key value in a hash map
  std::unordered_map<ull, unsigned long long> PageTableImplementationA;
  // the second implementation of page table as single level page table

  // this function will request the memory which is frame no from the memory
  // manager , and in return it will get the first alloted frame no in case of
  // requested size is larger than frame size we will have to handle that in
  // this as well , also is will also check of the requested virtual address
  // already exists in the page table
  void requestMemory(ull size, ull virtual_address);
  Task() {
    pageTableSize = PAGE_SIZE;
    noOfPages = VIRTUAL_MEM_SIZE / pageTableSize;
  }
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
  ull assignMemory(ull size, unsigned long long task_id);
};

// a universal instance of memory manager for keeping it seperate from the tasks
static MemoryManager mmInstance;
