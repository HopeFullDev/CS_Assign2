// #include <iostream>
// #include <unordered_map>
// #include <vector>
#include "bits/stdc++.h"

class Task {
public:
  unsigned long long task_id;
  std::unordered_map<unsigned long long int, unsigned long long int>
      PageTableImplementationA;
};

class MemoryManager {
public:
  unsigned long long freeMemory = 0x00000000000;
};
