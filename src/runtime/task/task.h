


#ifndef _FURIOUS_TASK_H_
#define _FURIOUS_TASK_H_ value

namespace furious
{

class Database;

using TaskFunc = void(float, // delta 
                      Database*, // database
                      void*, // userdata
                      uint32_t, // chunksize 
                      uint32_t,  // offset
                      uint32_t); // stride

struct Task 
{
  TaskFunc p_func;
};
  
}  

#endif /* ifndef _FURIOUS_TASK_H_ */
