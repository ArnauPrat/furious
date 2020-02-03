
#ifndef _FURIOUS_COMPILER_PRODUCER_H_
#define _FURIOUS_COMPILER_PRODUCER_H_ value

#include <stdio.h>


struct fcc_operator_t;

/**
 * \brief Implements the produce operation for generating code
 */
void
produce(FILE* fd,
        const fcc_operator_t* fcc_operator,
        bool parallel_stream);
  
#endif /* ifndef _FURIOUS_COMPILER_PRODUCE_VISITOR */
