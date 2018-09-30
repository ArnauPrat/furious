

#include "furious_runtime.h"
#include "furious.h"

namespace furious
{

Database* database;

extern void __furious_init(Database* database);
extern void __furious_frame(float delte, 
                     Database* database);
extern void __furious_release();

void init() 
{
  database = new Database{};
  __furious_init(database);
}

void release() 
{
  __furious_release();
  delete database;
}

void update(float delta)
{
  __furious_frame(delta, database);
}

Database* get_database()
{
  return database;
}
  
} /* furious
 */ 
