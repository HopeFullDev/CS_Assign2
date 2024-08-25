#include "config.h"
#include <boost/algorithm/string.hpp> // For boost::trim
#include <boost/config/detail/suffix.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
//

unsigned long long stringToAddress(const std::string& hexString) {
    std::stringstream ss(hexString);
    unsigned long long address;
    ss >> std::hex >> address;
    return address;
}

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

  //  Variables for keeping track of memory state multi level paging
  // variable to note the free memory available
  ull freeMemoryMultiLevel;
  // variable to note the total no of frames in the physical memory
  ull frameSizeMultiLevel;
  // variable to hold the next free frame no
  ull nextFreeFrameNoMultiLevel;
  // variable to track the free frames
  ull noOfFreeFramesMultiLevel;

  MemoryManager() {

    freeMemory = PHYSICAL_MEM_SIZE;

    frameSize = PAGE_SIZE;
    // intialising the free frame no
    nextFreeFrameNo = 0x00;
    //    finding the free variable
    noOfFreeFrames = PHYSICAL_MEM_SIZE / PAGE_SIZE;

    freeMemoryMultiLevel = PHYSICAL_MEM_SIZE;
    frameSizeMultiLevel = PAGE_SIZE;
    nextFreeFrameNoMultiLevel = 0x00;
    noOfFreeFramesMultiLevel = PHYSICAL_MEM_SIZE / PAGE_SIZE;
  }

  // vector to hold the details of which memory is alloted to which task id ,
  // how amany frames alloted , frame no of first frame alloted  and size
  // requested
  std::vector<MM_table> allotedMemoryTable;
  std::optional<ull> assignMemory(ull size, unsigned long long task_id);

  std::vector<MM_table> allotedMemoryTableMultiLevelPage;
  std::optional<ull> assignMemoryMultiLevelPage(ull size, ull task_id);
};
// The implementation of the memory alloting function for alloting memory to a
// task and updating the record in the vector table and the other member
// variables for multi level page
std::optional<ull>
MemoryManager::assignMemoryMultiLevelPage(unsigned long long size,
                                          unsigned long long task_id) {
  // check if the size requested is larger than available memory
  if (freeMemoryMultiLevel > size) {

    ull returnFrameNo = nextFreeFrameNoMultiLevel;

    //    adding the mm table entry in the vector
    allotedMemoryTableMultiLevelPage.push_back(
        {task_id, size, nextFreeFrameNoMultiLevel,
         static_cast<ull>(ceil((float)size / PAGE_SIZE))});
    //finding the no of frames to be alloted
    ull noOfFramesAlloted = static_cast<ull>(ceil((float)size / PAGE_SIZE));
    // updating the free Memory available
    freeMemoryMultiLevel -= (noOfFramesAlloted * PAGE_SIZE);
    noOfFreeFramesMultiLevel -= noOfFramesAlloted;
    // now updating the nextFreeFrameNoMultiLevel
    MM_table &lastElement = allotedMemoryTableMultiLevelPage.back();

    nextFreeFrameNoMultiLevel =
        lastElement.firstAllotedFrameNo +
        lastElement.noOfFramesAlloted /*ceil(size/PAGE_SIZE)+1*/;
    return returnFrameNo;
  }
  // in case if condidtion is alse then reaturn an error in the form of sending
  // max element of unsigned long long we have to handle this case while calling
  // this function
  return std::nullopt;
}

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
  ull pageHitImplementationA, pageHitImplementationB,
      pageHitImplementationClvl1, pageHitImplementationClvl2,
      pageHitImplementationClvl3;
  //  Variables to keep track of page miss/faults in different implementations
  ull pageFaultImplementationA, pageFaultImplementationB,
      pageFaultImplementationClvl1, pageFaultImplementationClvl2,
      pageFaultImplementationClvl3;

  // this is the first implementation of page table as key value in a hash map
  std::unordered_map<ull, ull> PageTableImplementationA;

  // the second implementation of page table as single level page table
  // we create the page table as a vector size = Page size
  std::vector<ull> PageTableImplementationB;

  // the multi level implementation for three level paging
  ull noOfEnteriesLevel1;
  ull noOfEnteriesLevel2;
  ull noOfEnteriesLevel3;
  // Level 1 page table holds pointer to level 2 page enteries
  //  std::vector<std::vector<ull *>*> PageTableImplementationCLevel1;
  // Level 2 page table which hold pointer to level 3 page enteries
  //  std::vector<std::vector<ull>*> PageTableImplementationCLevel2;
  // Level 3 page table which holds pointer to level 4 page enteries
  // std::vector<ull> PageTableImplementationCLevel3;
  std::vector<std::vector<std::vector<ull> *> *> PageTableImplementationCLevel1;

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
    PageTableImplementationB.resize((ull)noOfPages, 0);
    pageHitImplementationA = 0;
    pageHitImplementationB = 0;
    pageHitImplementationClvl1 = 0;
    pageHitImplementationClvl2 = 0;
    pageHitImplementationClvl2 = 0;
    pageFaultImplementationA = 0;
    pageFaultImplementationB = 0;
    pageFaultImplementationClvl1 = 0;
    pageFaultImplementationClvl2 = 0;
    pageFaultImplementationClvl3 = 0;
    noOfEnteriesLevel1 = VIRTUAL_MEM_SIZE / PAGE_TABLE_LEVEL_1_SIZE;
    noOfEnteriesLevel2 = PAGE_TABLE_LEVEL_2_SIZE;
    noOfEnteriesLevel3 = PAGE_TABLE_LEVEL_3_SIZE;
    PageTableImplementationCLevel1.resize(PAGE_TABLE_LEVEL_1_SIZE, nullptr);
  }
};
void Task::requestMemory(ull size, ull virtual_address) {
  //  i have to check if memory size is larger than single frame then it is
  //  possible that the second frame maight be already alloted so check that as
  //  well

  ull v_address = virtual_address;
  ull noOfFrameRequested = (ull)ceil(size / pageTableSize);
  for (uint32_t i = 0; i < noOfFrameRequested; i++) {
    // get the page number for the virtual address where
    ull pageNoOfVirtualAddress = v_address >> (ull)log2(pageTableSize);
    // we know that the difference between type A and Type B table is one will
    // be hashmap and other is a prealloted array first lets deal with
    // implemenation A

    //  Check if page index value aldready exists
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

      auto frameNo = mmInstance.assignMemory(size, task_id);

      if (!frameNo.has_value()) {
        // in case the requested size is larger than available memory
        std::cout << "Task Id: " << task_id << " requested " << size
                  << " in bytes but available memory is only "
                  << mmInstance.freeMemory << " in bytes" << std::endl;
        return;
      } else {
        // here we know that we have been alloted some frames
        ull initialFrameNo = frameNo.value();
        // now in case the requested memory is larger than single page we have
        // to handle that here
        ull noOfFramesAlloted = (size + PAGE_SIZE - 1) / PAGE_SIZE;

        for (ull i = 0; i < noOfFramesAlloted; i++) {
          PageTableImplementationA[pageNoOfVirtualAddress + i] =
              initialFrameNo + i;
          PageTableImplementationB[pageNoOfVirtualAddress + i] =
              initialFrameNo + 1;
        }
      }
    }

    v_address = v_address + (1ULL << (ull)log2(pageTableSize));
  }

  // lets check how many lvl3 pages we need for alloting the request Memory
  ull required_lvl3_pages = (ull)std::ceil(size / PAGE_SIZE);

  ull copy_of_virtual_addr = virtual_address;

  for (uint32_t i = 0; i < required_lvl3_pages; i++) {
    // translaing the virtual address for 3 level paging
    ull level1_index =
        (copy_of_virtual_addr >>
         (PAGE_TABLE_OFFSET_BITS + ((ull)log2(PAGE_TABLE_LEVEL_3_SIZE) +
                                    (ull)log2(PAGE_TABLE_LEVEL_2_SIZE)))) &
        ((1ULL << PAGE_TABLE_OFFSET_BITS) - 1);
    //  this index will store point to a level 2 page here each entry denotes
    //  4KB physical space
    ull level2_index =
        (copy_of_virtual_addr >>             (PAGE_TABLE_OFFSET_BITS + ((ull)log2(PAGE_TABLE_LEVEL_3_SIZE))) ) &       (  (1ULL << PAGE_TABLE_OFFSET_BITS) - 1) ;
    // this index point to value in level1_index here each entry denotes 4MB
    // physical space
    ull level3_index = ((copy_of_virtual_addr >> PAGE_TABLE_OFFSET_BITS) &
                        ((1ULL << PAGE_TABLE_OFFSET_BITS) - 1));

    // check if lvl 2 table for the index exists or not
    if (!PageTableImplementationCLevel1[level1_index]) {
      pageFaultImplementationClvl1++;
      PageTableImplementationCLevel1[level1_index] =
          new std::vector<std::vector<ull> *>(PAGE_TABLE_LEVEL_2_SIZE, nullptr);
    } else {
      pageHitImplementationClvl1++;
    }
    // check if lvl 3 table for the index exists or not , this lvl 3 holds the
    // frame no
    if (!(*PageTableImplementationCLevel1[level1_index])[level2_index]) {
      pageFaultImplementationClvl2++;
      (*PageTableImplementationCLevel1[level1_index])[level2_index] =
          new std::vector<ull>(PAGE_TABLE_LEVEL_3_SIZE, 0);
    } else {
      pageHitImplementationClvl2++;
    }
    // check if there is a frame no present in the lvl3 table at the lvl 3 index
    if ((*(*PageTableImplementationCLevel1[level1_index])[level2_index])
            [level3_index] != 0) {
      // means there is a frame present at the index
      // also the frame no at this index should be a frame no which will
      // correspond to a 4KB page in physical memory
      pageHitImplementationClvl3++;
    } else {
      pageFaultImplementationClvl3++;
      // request the frame from memory manager
      auto frameNo = mmInstance.assignMemoryMultiLevelPage(PAGE_SIZE, task_id);
      if (frameNo.has_value()) {
        (*(*PageTableImplementationCLevel1[level1_index])[level2_index])
            [level3_index] = frameNo.value();
      } else {
        // in case the requested size is larger than available memory
        std::cout << "Task Id: " << task_id << " requested " << size
                  << " in bytes but available memory is only "
                  << mmInstance.freeMemory << " in bytes FOR MULTI LEVEL PAGING"
                  << std::endl;
        return;
      }
      copy_of_virtual_addr =
          copy_of_virtual_addr + (1ULL << (ull)log2(PAGE_TABLE_LEVEL_3_SIZE));
    }
  }

  /*
   * Test of three level paging idea
    std::vector<ull> lvl3(PAGE_TABLE_LEVEL_3_SIZE,0);
    std::vector<std::vector<ull>* > lvl2(PAGE_TABLE_LEVEL_2_SIZE, nullptr);
    std::vector<std::vector <std::vector<ull>* > *>
    lvl1(PAGE_TABLE_LEVEL_1_SIZE,nullptr);

    lvl2[0] = &lvl3;
  */

  return;
}

// Main Function

int main() {
  std::vector<Task> holdsAllTasks;
  std::string inputFilename;
  std::cout << "Enter the file name with the extension .txt :" << std::endl;
  std::cin >> inputFilename;
  std::ifstream file(inputFilename);
  if (!file) {
    std::cerr << "Unable to open file." << std::endl;
    return 1; // Return error code
  }
   // Test string
    std::string test_string;
    std::getline(file, test_string);
  
  while (!test_string.empty()) {
    std::regex pattern(R"(T(\d+):0x([0-9A-Fa-f]+):(\d+)(K|M)B)");

   
    boost::trim(test_string);

    // Declare a match object to hold the results of the search

    std::smatch match;
    ull task_id;
    ull memory_address;
    ull memory_size;
    
    // Perform the regex search with capturing groups
    if (std::regex_search(test_string, match, pattern)) {
      // match[1] is the first capture group (Task ID)
      task_id = std::stoull(match[1].str());

      // match[2] is the second capture group (Memory Address in Hex)
      memory_address = stringToAddress(match[2].str());

      // match[3] is the third capture group (Memory Size in KB)
      memory_size = std::stoull(match[3].str());
      std::cout<<"task_id: "<<match[1].str()<<"\tMemory_address " <<match[2].str()<<"\t MemorySize "<<match[3].str()<<std::endl;
            // printf("task_id: %s\tMemory_address %s\t MemorySize %s\t\n", , match[2].str(), match[3].str());
      printf("task_id: %llu\tMemory_address %llu\t MemorySize %llu\t\n", task_id, memory_address, memory_size);
    }

    for (ull i = 0; i < holdsAllTasks.size(); i++) {
      if (holdsAllTasks[i].task_id == task_id) {
        holdsAllTasks[i].requestMemory(memory_size, memory_address);
        break;
      }

      else {
        Task newtask;
        newtask.task_id = task_id;
        holdsAllTasks.push_back(newtask);
        newtask.requestMemory(memory_size, memory_address);
      }
    }
    std::getline(file, test_string);
  }
  std::cout
      << "Check the Following Memory Details of Processes and Memory Manager :"
      << std::endl;
  std::cout << "As per the Assignment we have the following assumptions :"
            << std::endl;
  std::cout << "Virtual Memory size is  : " << VIRTUAL_MEM_SIZE << " Bytes"<< std::endl;
  std::cout << "Physical Memory size is : " << PHYSICAL_MEM_SIZE << " Bytes"<< std::endl;
  std::cout << "Page Size size is       : " << PAGE_SIZE << " Bytes"<< std::endl;

  for (ull i = 0; i < holdsAllTasks.size(); i++) {
    std::cout << "The Task ID - T" << holdsAllTasks[i].task_id
              << " has the following details : " << std::endl;
    std::cout << "Page Table Size is : " << holdsAllTasks[i].pageTableSize
              << std::endl;
    std::cout << "Number of Page hits in Implementation A (hash_map) are  = :"
              << holdsAllTasks[i].pageHitImplementationA << std::endl;
    std::cout << "Number of Page hits in Implementation B (Single Level Page "
                 "Table) are  = :"
              << holdsAllTasks[i].pageHitImplementationB << std::endl;
    std::cout << "Number of Page hits in Implementation C (Multi Level Page "
                 "Table) are  = :"
              << holdsAllTasks[i].pageHitImplementationClvl1 +
                     holdsAllTasks[i].pageHitImplementationClvl2 +
                     holdsAllTasks[i].pageHitImplementationClvl3
              << std::endl;

    std::cout << "Number of Page Misses in Implementation A (hash_map) are  = :"
              << holdsAllTasks[i].pageFaultImplementationA << std::endl;
    std::cout << "Number of Page Misses in Implementation B (hash_map) are  = :"
              << holdsAllTasks[i].pageFaultImplementationB << std::endl;
    std::cout << "Number of Page Misses in Implementation C (hash_map) are  = :"
              << holdsAllTasks[i].pageFaultImplementationClvl1 +
                     holdsAllTasks[i].pageFaultImplementationClvl2 +
                     holdsAllTasks[i].pageFaultImplementationClvl3
              << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Memory Details for Implementation as Map and Single Level Page Table" <<std::endl;
  std::cout << " Free Memory : " << mmInstance.freeMemory << " Bytes\n";
  std::cout << " Memory Used : " << (PHYSICAL_MEM_SIZE - mmInstance.freeMemory)
            << "  Bytes\n";

        std::cout << std::endl;
  std::cout << "Memory Details for Three Level Page Table Implementation " <<std::endl;
  std::cout << " Free Memory : " << mmInstance.freeMemoryMultiLevel << " Bytes\n";
  std::cout << " Memory Used : " << (PHYSICAL_MEM_SIZE - mmInstance.freeMemoryMultiLevel)
            << "  Bytes\n";
}
