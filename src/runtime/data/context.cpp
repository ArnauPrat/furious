


#include "context.h"

namespace furious 
{

Context::Context(float dt, 
                 Database* database, 
                 void* user_data) : 
  m_dt(dt), 
  p_user_data(user_data),
  p_database(database)
  {
  }

}
