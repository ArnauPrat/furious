

#ifndef _FURIOUS_BACKEND_H_
#define _FURIOUS_BACKEND_H_ value
namespace furious {

class Workload;
class Database;

class Backend {
public:
  Backend() = default;
  virtual ~Backend() = default;

  /**
   * @brief Generates the execution plan and compiles the query based on the
   * workload 
   *
   * @param workload The workload to compile the query for
   * @param database The database to compile the query for
   */
  virtual void compile(Workload* workload, 
                       Database* database) = 0;

  /**
   * @brief Runs the compiled query 
   *
   * @param delta The delta time to pass to the execution context.
   */
  virtual void run(float delta) = 0;
};
  
} /* furious */ 
#endif /* ifndef _FURIOUS_BACKEND_H_ */
