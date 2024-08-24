#include "config.h"
#include <bits/stdc++.h>
#include <iostream>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>  // For boost::trim

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

    ull returnFrameNo = nextFreeFrameNo;

    //    adding the mm table entry in the vector
    allotedMemoryTable.push_back(
        {task_id, size, nextFreeFrameNo,
         static_cast<ull>(ceil((float)size / PAGE_SIZE))});
    ull noOfFramesAlloted = static_cast<ull>(ceil((float)size / PAGE_SIZE));
    // updating the free Memory available
    freeMemory -= (noOfFramesAlloted * PAGE_SIZE);
    noOfFreeFrames -= noOfFramesAlloted;
    // now updating the nextFreeFrameNo
    MM_table &lastElement = allotedMemoryTable.back();

    nextFreeFrameNo = lastElement.firstAllotedFrameNo +
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
    task_id = 0;
    pageTableSize = PAGE_SIZE;
    noOfPages = VIRTUAL_MEM_SIZE / pageTableSize;
    PageTableImplementationB.resize((ull)noOfPages);
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

  // we know that the difference between type A and Type B table is one will be
  // hashmap and other is a prealloted array first lets deal wiht implemenattion
  // A
  //  Check if page aldready exists
  if (PageTableImplementationA.find(pageNoOfVirtualAddress) !=
      PageTableImplementationA.end()) {
    pageHitImplementationA++;

    pageHitImplementationB++;
    return;
  }
  // for implemantation A the page does not exist. so request the page
  else {
    pageFaultImplementationA++;
    pageFaultImplementationB++;

    auto frameNo = mmInstance.assignMemory(size, this->task_id);

    if (!frameNo.has_value()) {
      // in case the requested size is larger than available memory
      std::cout << "Task Id: " << task_id << " requested " << size
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
        PageTableImplementationB[pageNoOfVirtualAddress + i] =
            initialFrameNo + 1;
      }
    }
  }
  return;
}

// Main Function

int main()
{
 vector<Task> holdsAllTasks;
 std:: string inputFilename;
 std::cout<<"Enter the file name with the extension .text :"<<endl;
 std::cin>>inputFilename;
   std::ifstream file("inputFilename");
  if (!file) {
        std::cerr << "Unable to open file." << std::endl;
        return 1;  // Return error code  
  }
   std:: string line; 
   while(std::getline(file, line))
     {
  std::regex pattern(R"(T(\d+):0x([0-9A-Fa-f]+):(\d+)KB)");

  // Test string
  std::string test_string = std::getline(file, line);
  boost::trim(test_string); 
       
  // Declare a match object to hold the results of the search
       
  std::smatch match;
  ull task_id;
  ull memory_address;
  ull memory_size;
  // Perform the regex search with capturing groups
  if (std::regex_search(test_string, match, pattern)) {
    // match[1] is the first capture group (Task ID)
    task_id = std::stoull(match[1]);

    // match[2] is the second capture group (Memory Address in Hex)
    memory_address = std::stoull(match[2]);

    // match[3] is the third capture group (Memory Size in KB)
    memory_size = std::stoull(smatch[3]);
  }
       
  for(ull i=0;i<holdsALLTasks.size();i++)
    {
 if(holdsAllTasks[i].task_id==task_id )
 holdsAllTasks[i].requestMemory(memory_size,memory_address);
      break:
}
 else
    {
      Task newtask;
      newtask.task_id= task_id;
      holdsAllTasks.push_back(newtask);
      newtask.requestMemory(memory_size,memory_address);
     }

  cout<<"Check the Following Memory Details of Processes nd Memory Manager :"<<endl;
  cout<<"As per the Assignment we have the following assumptions :"<<endl;
  cout<<"Virtual Memory size is  : 4GB"<<endl;
  cout<<"Physical Memory size is : 4GB"<<endl;
  cout<<"Page Size size is       : 4GB"<<endl;
  for(ull i=0;i<holdsALLTasks.size();i++)
    {
    std:: cout<<"The Task ID - T"<< holdsAllTasks[i].task_id << " has the following details : " <<endl<<;
    std:: cout<<"Page Table Size is : "<<holdsAllTasks[i].pageTableSize<<endl;
    std:: cout<<"Number of Page hits in Implementation A (hash_map) are  = :"<<holdsAllTasks[i].pageHitImplementationA<<endl;
    std:: cout<<"Number of Page hits in Implementation B (Single Level Page Table) are  = :"<<holdsAllTasks[i].pageHitImplementationB<<endl; 
    std:: cout<<"Number of Page hits in Implementation C (Multi Level Page Table) are  = :"<<holdsAllTasks[i].pageHitImplementationC<<endl; 
    std:: cout<<"Number of Page Misses in Implementation A (hash_map) are  = :"<<holdsAllTasks[i].pageFaultImplementationA<<endl;
    std:: cout<<"Number of Page Misses in Implementation B (hash_map) are  = :"<<holdsAllTasks[i].pageFaultImplementationB<<endl;
    std::  cout<<"Number of Page Misses in Implementation C (hash_map) are  = :"<<holdsAllTasks[i].pageFaultImplementationC<<endl<<endl;
    }
       MemoryManager display; 
      std:: cout<<endl<<endl;
      std:: cout<<"Total Memory Size : 4 GB";
      std:: cout<<" Free Memory : "<<display.freeMemory<<" Bytes";
      std:: cout<<" Memory Used : "<<(2^32 - display.freeMemory) <<"  Bytes";
    
}

