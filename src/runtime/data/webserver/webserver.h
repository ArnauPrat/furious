


#ifndef _FURIOUS_WEBSERVER_H_
#define _FURIOUS_WEBSERVER_H_ value

#include "../../../common/types.h"
#include "../../../common/string_builder.h"

#include <string>

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
        const std::string& address,
        const std::string& port);

  void 
  stop();


  Database*     p_database; 
  std::thread*  p_thread;
  uint32_t      m_table_infos_capacity;
  TableInfo*    m_table_infos;

  StringBuilder m_builder;
}; 
}/* furious */ 
#endif /* ifndef _FURIOUS_WEBSERVER_H_ */
