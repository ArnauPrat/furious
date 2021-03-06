

#ifndef _FDB_COMPILER_CODEGEN_H_
#define _FDB_COMPILER_CODEGEN_H_ value

#include "common/platform.h"


struct fcc_exec_plan_t;

/**
 * \brief This function generates the code for a given execution plan and prints
 * it to the provided filename. The technique used to generate the code is that
 * published in the paper:
 *    @article{neumann2011efficiently,
 *      title={Efficiently compiling efficient query plans for modern hardware},
 *      author={Neumann, Thomas},
 *      journal={Proceedings of the VLDB Endowment},
 *      volume={4},
 *      number={9},
 *      pages={539--550},
 *      year={2011},
 *      publisher={VLDB Endowment}
 *    }
 *
 * \param exec_plan The execution plan to generate the code for
 * \param filename The file where the code is going to be written.
 */
int32_t 
fcc_generate_code(const fcc_exec_plan_t* exec_plan,
                  const fcc_exec_plan_t* post_exec_plan,
                  const char* filename,
                  const char* include_file);





#endif /* ifndef _FDB_COMPILER_CODEGEN_H_ */
