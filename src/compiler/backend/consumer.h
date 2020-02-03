
#ifndef _FURIOUS_COMPILER_CONSUMER_H_
#define _FURIOUS_COMPILER_CONSUMER_H_ value

#include <stdio.h>

struct fcc_operator_t;

/**
 * \brief Visitor that implements the consume operation for generating code
 */
void
consume(FILE* fd,
        const fcc_operator_t* op,
        const char* source,
        const fcc_operator_t* caller);

#endif
