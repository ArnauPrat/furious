


#ifndef _FURIOUS_WEBSERVER_H_
#define _FURIOUS_WEBSERVER_H_ value

#include "../../../common/types.h"
#include "../../../common/str_builder.h"

namespace std
{
class thread;
}


namespace furious
{

struct Database;
struct TableInfo;

struct WebServer
{

  WebServer();
  ~WebServer();

  void
  start(Database* database,
        const char* address,
        const char* port);

  void 
  stop();


  Database*     p_database; 
  std::thread*  p_thread;
  uint32_t      m_table_infos_capacity;
  TableInfo*    m_table_infos;

  str_builder_t m_builder;
}; 
}/* furious */ 
#endif /* ifndef _FURIOUS_WEBSERVER_H_ */
