
#include "utils.h"
#include <functional>

namespace furious  {

uint64_t hash(const std::string& str) {
  return std::hash<std::string>{}(str);
}
  
} /* furious  */ 
