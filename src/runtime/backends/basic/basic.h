

#ifndef _FURIOUS_BASIC_H_
#define _FURIOUS_BASIC_H_ value

#include "../../backend.h"

namespace furious {

class Basic : public Backend {
public:
  Basic() = default;
  virtual ~Basic() = default;

  void compile(Workload* workload,
               Database* database) override;

  void run(float delta) override;

private:
  Workload* p_workload = nullptr;
  Database* p_database = nullptr;
};
  
} /* furious */ 
#endif /* ifndef _FURIOUS_BASIC_H_ */

