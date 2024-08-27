#include "config.h"
#include <boost/algorithm/string.hpp> // For boost::trim
#include <boost/config/detail/suffix.hpp>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

unsigned long long stringToAddress(const std::string &hexString) {
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

  // vector to hold the details of which memory is alloted to which task id ,
  // how amany frames alloted , frame no of first frame alloted  and size
  // requested
  std::vector<MM_table> allotedMemoryTable;
  std::optional<ull> assignMemory(ull size, unsigned long long task_id);

  std::vector<MM_table> allotedMemoryTableMultiLevelPage;
  std::optional<ull> assignMemoryMultiLevelPage(ull size, ull task_id);

  MemoryManager() {

    freeMemory = PHYSICAL_MEM_SIZE;

    frameSize = PAGE_SIZE;
    // intialising the free frame no
    nextFreeFrameNo = 0x01;
    //    finding the free variable
    noOfFreeFrames = PHYSICAL_MEM_SIZE / PAGE_SIZE;

    freeMemoryMultiLevel = PHYSICAL_MEM_SIZE;
    frameSizeMultiLevel = PAGE_SIZE;
    nextFreeFrameNoMultiLevel = 0x01;
    noOfFreeFramesMultiLevel = PHYSICAL_MEM_SIZE / PAGE_SIZE;
  }
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
         static_cast<ull>(ceil(size / (float)PAGE_SIZE))});
    // finding the no of frames to be alloted
    ull noOfFramesAlloted = static_cast<ull>(ceil(size / (float)PAGE_SIZE));
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
    pageHitImplementationClvl3 = 0;
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

  /*~Task() {
    std::cout << "Destroying Task " << task_id << std::endl;
    for (size_t i = 0; i < PageTableImplementationCLevel1.size(); ++i) {
      auto level1_entry = PageTableImplementationCLevel1[i];
      if (level1_entry) {
        for (size_t j = 0; j < level1_entry->size(); ++j) {
          auto level2_entry = (*level1_entry)[j];
          if (level2_entry) {
            std::cout << "Deleting level 3 table at [" << i << "][" << j << "]"
                      << std::endl;
            delete level2_entry;
          }
        }
        std::cout << "Deleting level 2 table at [" << i << "]" << std::endl;
        delete level1_entry;
      }
    }
    PageTableImplementationCLevel1.clear();
  }*/
};
void Task::requestMemory(ull size, ull virtual_address) {
  //  i have to check if memory size is larger than single frame then it is
  //  possible that the second frame maight be already alloted so check that as
  //  well

  ull v_address = virtual_address;

  ull noOfFrameRequested = ceil(size / (float)pageTableSize);

  for (int i = 0; i < (int)noOfFrameRequested; i++) {
    // get the page number for the virtual address where
    ull pageNoOfVirtualAddress = v_address >> (ull)log2(pageTableSize);

    // we know that the difference between type A and Type B table is one will
    // be hashmap and other is a prealloted array first lets deal with
    // implemenation A

    //  Check if page index value aldready exists
    if (PageTableImplementationA.count(pageNoOfVirtualAddress)) {
      pageHitImplementationA++;
      // printf("\nif page already exists %llx \n", pageNoOfVirtualAddress);
      pageHitImplementationB++;
    }
    // for implemantation A the page does not exispageNoOfVirtualAddresst. so
    // request the page
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
        PageTableImplementationA[pageNoOfVirtualAddress] = initialFrameNo;
        PageTableImplementationB[pageNoOfVirtualAddress] = initialFrameNo;
      }
    }

    v_address = v_address + (1ULL << (ull)log2(pageTableSize));
  }

  // lets check how many lvl3 pages we need for alloting the request Memory
  ull required_lvl3_pages = ceil(size / (float)PAGE_SIZE);
  // printf("\n\nTask Id : %llu , loop size : %llu\n", task_id,
  //  required_lvl3_pages);
  ull copy_of_virtual_addr = virtual_address;
  // printf(" Virtual address at start %llx \n", copy_of_virtual_addr);

  for (uint32_t i = 0; i < required_lvl3_pages; i++) {
    // printf("In Loop Task Id : %llu , loop size : %llu\n", task_id,
    //        required_lvl3_pages);

    // translaing the virtual address for 3 level paging
    ull level1_index =
        (copy_of_virtual_addr >>
         (PAGE_TABLE_OFFSET_BITS + ((ull)log2(PAGE_TABLE_LEVEL_3_SIZE) +
                                    (ull)log2(PAGE_TABLE_LEVEL_2_SIZE)))) &
        ((1ULL << PAGE_TABLE_OFFSET_BITS) - 1);
    //  this index will store point to a level 2 page here each entry denotes
    //  4KB physical space
    ull level2_index =
        (copy_of_virtual_addr >>
         (PAGE_TABLE_OFFSET_BITS + ((ull)log2(PAGE_TABLE_LEVEL_3_SIZE)))) &
        ((1ULL << PAGE_TABLE_OFFSET_BITS) - 1);
    // this index point to value in level1_index here each entry denotes 4MB
    // physical space
    ull level3_index = ((copy_of_virtual_addr >> PAGE_TABLE_OFFSET_BITS) &
                        ((1ULL << PAGE_TABLE_OFFSET_BITS) - 1));

    // printf("\nVirtual Address %llx\t, Lvl1 index %llu\t Lvl2 index %llu\t
    // Lvl3 "
    //        "%llu\n",
    //        copy_of_virtual_addr, level1_index, level2_index, level3_index);

    // check if lvl 2 table for the index exists or not
    if (!PageTableImplementationCLevel1[level1_index]) {
      pageFaultImplementationClvl1++;
      // printf("Page Fault LVL 1");

      PageTableImplementationCLevel1[level1_index] =
          new std::vector<std::vector<ull> *>(PAGE_TABLE_LEVEL_2_SIZE, nullptr);

      // printf(" pointer to lvl 2 %llu\n",
      //        PageTableImplementationCLevel1[level1_index]);
    } else {
      pageHitImplementationClvl1++;
    }
    // check if lvl 3 table for the index exists or not , this lvl 3 holds
    // the
    // frame no
    if (!(*PageTableImplementationCLevel1[level1_index])[level2_index]) {
      pageFaultImplementationClvl2++;
      (*PageTableImplementationCLevel1[level1_index])[level2_index] =
          new std::vector<ull>(PAGE_TABLE_LEVEL_3_SIZE, 0);

      // printf("Page Fault LVL 2");
      // printf(" pointer to lvl 3 %llu\n",
      //        (*PageTableImplementationCLevel1[level1_index])[level2_index]);
    } else {
      pageHitImplementationClvl2++;
    }
    // check if there is a frame no present in the lvl3 table at the lvl 3
    // index

    if ((*(*PageTableImplementationCLevel1[level1_index])[level2_index])
            [level3_index] == 0) {
      // std::cout << "Before incrementing the page fault "
      //           << pageHitImplementationClvl3 << std::endl;
      pageFaultImplementationClvl3++;
      // request the frame from memory manager
      auto frameNo = mmInstance.assignMemoryMultiLevelPage(PAGE_SIZE, task_id);
      if (frameNo.has_value()) {
        (*(*PageTableImplementationCLevel1[level1_index])[level2_index])
            [level3_index] = frameNo.value();

        // printf(
        //     "Page Fault At LVL 3 :  %llu\tindex %llu\t Value in table
        //     %llu\n", frameNo.value(), level3_index,
        //     (*(*PageTableImplementationCLevel1[level1_index])[level2_index])
        //         [level3_index]);
      } else {
        // in case the requested size is larger than available memory
        std::cout << "Task Id: " << task_id << " requested " << size
                  << " in bytes but available memory is only "
                  << mmInstance.freeMemory << " in bytes FOR MULTI LEVEL PAGING"
                  << std::endl;
        return;
      }

      // std::cout << "task id\t" << task_id << "  lvl3 Index " << level3_index
      //           << " Page Faults : " << (pageFaultImplementationClvl3) <<
      //           "\n";

    } else {

      // means there is a frame present at the index
      // also the frame no at this index should be a frame no which will
      // correspond to a 4KB page in physical memory
      // std::cout << "task id\t" << task_id << "  lvl3 Index " << level3_index
      //           << " Page Hits : " << (pageHitImplementationClvl3 + 1) <<
      //           "\n";
      pageHitImplementationClvl3++;
      // printf("Page HIT At LVL 3 : %llu \t level3 index %llu\n ",
      //        (*(*PageTableImplementationCLevel1[level1_index])[level2_index])
      //            [level3_index],
      //        level3_index);
    }
    copy_of_virtual_addr =
        copy_of_virtual_addr + (1ULL << PAGE_TABLE_OFFSET_BITS);
    // printf(" updated Virtual address in loop %llx \n", copy_of_virtual_addr);
  }

  /*
    // Test of three level paging idea
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
  // std::getline(file, test_string);

  while (std::getline(file, test_string)) {
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

      if (match[4].str()[0] == 'K') {
        memory_size = memory_size << 10;
      } else {
        memory_size = memory_size << 20;
      }
    }
    if (holdsAllTasks.empty()) {
      Task newtask;
      newtask.task_id = task_id;
      newtask.requestMemory(memory_size, memory_address);
      holdsAllTasks.push_back(newtask);
    } else {
      bool taskFound = false;
      for (auto &existingTask : holdsAllTasks) {
        if (existingTask.task_id == task_id) {
          existingTask.requestMemory(memory_size, memory_address);
          taskFound = true;
          break;
        }
      }
      if (!taskFound) {
        Task newtask;
        newtask.task_id = task_id;
        newtask.requestMemory(memory_size, memory_address);
        holdsAllTasks.push_back(newtask);
      }
    }
  }
  file.close();

  std::cout << "\n\n\n\nCheck the Following Memory Details of Processes and "
               "Memory Manager :"
            << std::endl;
  std::cout << "As per the Assignment we have the following assumptions :"
            << std::endl;
  std::cout << "Virtual Memory size is  : " << VIRTUAL_MEM_SIZE << " Bytes"
            << std::endl;
  std::cout << "Physical Memory size is : " << PHYSICAL_MEM_SIZE << " Bytes"
            << std::endl;
  std::cout << "Page Size size is       : " << PAGE_SIZE << " Bytes"
            << std::endl;

  ull totalPageHitsImplementationA = 0;
  ull totalPageHitsImplementationB = 0;
  ull totalPageHitsImplementationC = 0;
  ull totalPageFaultsImplementationA = 0;
  ull totalPageFaultsImplementationB = 0;
  ull totalPageFaultsImplementationC = 0;

  for (ull i = 0; i < holdsAllTasks.size(); i++) {
    std::cout << "The Task ID - T" << holdsAllTasks[i].task_id
              << " has the following details : " << std::endl;
    std::cout << "Page Table Size is : " << holdsAllTasks[i].pageTableSize
              << std::endl;
    std::cout << "hash map Size is : "
              << holdsAllTasks[i].PageTableImplementationA.size() << std::endl;

    std::cout << "Number of Page hits in Implementation A (hash_map) are  = :"
              << holdsAllTasks[i].pageHitImplementationA << std::endl;
    std::cout << "Number of Page hits in Implementation B (Single Level Page "
                 "Table) are  = :"
              << holdsAllTasks[i].pageHitImplementationB << std::endl;
    std::cout << "Number of Page hits in Implementation C (Multi Level Page "
                 "Table) are  = :"
              << holdsAllTasks[i].pageHitImplementationClvl3 << std::endl;

    std::cout << "Number of Page Misses in Implementation A (hash_map) are  = :"
              << holdsAllTasks[i].pageFaultImplementationA << std::endl;
    std::cout << "Number of Page Misses in Implementation B (Single Level Page "
                 "Table) are  = :"
              << holdsAllTasks[i].pageFaultImplementationB << std::endl;
    std::cout << "Number of Page Misses in Implementation C (Multi Level Page "
                 "Table) are  = :"
              << holdsAllTasks[i].pageFaultImplementationClvl3 << std::endl;
    totalPageHitsImplementationA =
        totalPageHitsImplementationA + holdsAllTasks[i].pageHitImplementationA;
    totalPageHitsImplementationB =
        totalPageHitsImplementationB + holdsAllTasks[i].pageHitImplementationB;
    totalPageHitsImplementationC = totalPageHitsImplementationC +
                                   holdsAllTasks[i].pageHitImplementationClvl3;
    totalPageFaultsImplementationA = totalPageFaultsImplementationA +
                                     holdsAllTasks[i].pageFaultImplementationA;
    totalPageFaultsImplementationB = totalPageFaultsImplementationB +
                                     holdsAllTasks[i].pageFaultImplementationB;
    totalPageFaultsImplementationC =
        totalPageFaultsImplementationC +
        holdsAllTasks[i].pageFaultImplementationClvl3;
  }

  std::cout << std::endl;
  std::cout
      << "Memory Details for Implementation as Map and Single Level Page Table"
      << std::endl;
  std::cout << " Free Memory : " << mmInstance.freeMemory << " Bytes\n";
  std::cout << " Memory Used : " << (PHYSICAL_MEM_SIZE - mmInstance.freeMemory)
            << "  Bytes\n";
  std::cout << "Memory Details for Three Level Page Table Implementation "
            << std::endl;
  std::cout << " Free Memory : " << mmInstance.freeMemoryMultiLevel
            << " Bytes\n";
  std::cout << " Memory Used : "
            << (PHYSICAL_MEM_SIZE - mmInstance.freeMemoryMultiLevel)
            << "  Bytes\n";

  std::cout << "Total No of Page Hits Map Implementation A : "
            << totalPageHitsImplementationA << std::endl;
  std::cout << "Total No of Page Hits Single Level Implementation B :  "
            << totalPageHitsImplementationB << std::endl;
  std::cout << "Total No of Page Hits Three Level Implementation C : "
            << totalPageHitsImplementationC << std::endl;

  std::cout << "Total No of Page Miss Map Implementation A : "
            << totalPageFaultsImplementationA << std::endl;
  std::cout << "Total No of Page Miss Single Level Implementation B :  "
            << totalPageFaultsImplementationB << std::endl;
  std::cout << "Total No of Page Miss Three Level Implementation C : "
            << totalPageFaultsImplementationC << std::endl;

  std::cout << std::endl;
  // for (auto i = 0; i < holdsAllTasks.size(); i++) {
  //   std::cout << "task Id" << holdsAllTasks[i].task_id << std::endl;
  // }
  // holdsAllTasks.clear();
}
