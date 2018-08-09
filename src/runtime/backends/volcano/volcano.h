

#ifndef _FURIOUS_VOLCANO_H_
#define _FURIOUS_VOLCANO_H_ value


#include "../../../common/common.h"
#include "../../backend.h"

namespace furious {

class Workload;
class Database;
class TempBuffer;
struct ExecutionPlan;

class Volcano : public Backend {
public:
  Volcano();
  virtual ~Volcano();

  void compile(Workload* workload,
               Database* database) override;

  void run(float delta) override;


private:

  void reset();

  ExecutionPlan*  p_exec_plan;
  TempBuffer*     p_temp_buffer;

};
  
} /* furious */ 
#endif /* ifndef _FURIOUS_VOLCANO_H_ */
