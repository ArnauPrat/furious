
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include <sstream>
#include <vector>
#include <string>

namespace furious 
{

class FccOperator;

void 
consume(std::stringstream& output,
        const FccOperator* op,
        const std::string& source,
        const std::vector<std::string>& types,
        const FccOperator* caller);

void 
produce(std::stringstream& output,
        const FccOperator* op);

} /* furious */ 

#endif
