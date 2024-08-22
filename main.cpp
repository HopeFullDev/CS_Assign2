// #include <iostream>
// #include <unordered_map>
// #include <vector>
#include "bits/stdc++.h"
#include "config.h"

#define ull unsigned long long
class Task {
public:
  unsigned long long task_id;
  std::unordered_map<unsigned long long, unsigned long long>
      PageTableImplementationA;

  void requestMemory(unsigned long long size,
                     unsigned long long virtual_address);
};

class MemoryManager {
public:
  unsigned long long freeMemory = PHYSICAL_MEM_SIZE;
  unsigned long long frameSize = 0x000000000;
  void assignMemory(unsigned long long size, unsigned long long task_id);
};
