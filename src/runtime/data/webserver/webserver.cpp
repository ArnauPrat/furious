


#include "webserver.h"

#include <thread>


namespace furious
{

bool keep_running = true;

void
server_thread_handler(Database* database,
                      const std::string& address,
                      const std::string& port)
{
  while(keep_running)
  {
  }
}

void
WebServer::start(Database* database,
                 const std::string& address,
                 const std::string& port)
{
  p_database = database;
  p_thread = new std::thread(server_thread_handler, 
                             p_database,
                             address,
                             port);
}

void
WebServer::stop()
{
  keep_running = false;
  p_thread->join();
  delete p_thread;
}
  
} /* furious
 */ 
