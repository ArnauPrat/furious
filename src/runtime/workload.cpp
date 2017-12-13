
#include "workload.h"

namespace furious {

Workload::Workload() {
}

Workload::~Workload() {
  for (auto entry : m_systems) {
    delete entry.second;
  }
}

} /* furious */ 
