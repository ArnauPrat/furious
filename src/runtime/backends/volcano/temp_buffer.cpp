
#include "temp_buffer.h"

namespace furious {

TempBuffer::TempBuffer(size_t num_elements,
                       size_t num_components) :
  m_num_elements{num_elements},
  m_num_components{num_components} {

    p_data = new char[m_num_elements*m_num_components*sizeof(void*)];

  }

TempBuffer::~TempBuffer() {
  delete [] p_data;
}
  
} /* furious */ 
