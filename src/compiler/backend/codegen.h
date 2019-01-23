

#ifndef _FURIOUS_COMPILER_CODEGEN_H_
#define _FURIOUS_COMPILER_CODEGEN_H_ value

#include <string>

namespace furious {

struct FccExecPlan;
class ProduceVisitor;
class ConsumeVisitor;
class FccOperator;



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
void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename);

  
} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CODEGEN_H_ */
