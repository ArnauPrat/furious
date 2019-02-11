


#ifndef _FURIOUS_WEBSERVER_H_
#define _FURIOUS_WEBSERVER_H_ value

#include <string>

namespace std
{
class thread;
}


namespace furious
{

struct Database;

struct WebServer
{

  void
  start(Database* database,
        const std::string& address,
        const std::string& port);

  void 
  stop();

  Database* p_database; 
  std::thread* p_thread;
  
}; 
}/* furious */ 
#endif /* ifndef _FURIOUS_WEBSERVER_H_ */
