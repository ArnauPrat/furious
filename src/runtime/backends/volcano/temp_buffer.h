

#ifndef _FURIOUS_TEMP_BUFFER_H_
#define _FURIOUS_TEMP_BUFFER_H_ value

#include "../../../common/common.h"

namespace furious {

struct TempBuffer {

  TempBuffer(size_t num_elements,
             size_t num_components);

  ~TempBuffer();

  char* p_data;
  size_t m_num_elements;
  size_t m_num_components;
};


  
} /* furious */ 
#endif /* ifndef _FURIOUS_TEMP_BUFFER_H_ */
