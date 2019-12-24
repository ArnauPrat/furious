



#include "../database.h"
#include "webserver.h"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <thread>


namespace furious
{

bool keep_running = true;

bool
generate_json_from_infos(WebServer* webserver, uint32_t num_infos)
{


  return true;
}

void
generate_json(WebServer* webserver)
{
  uint32_t num_tables = webserver->p_database->num_tables();
  if(num_tables > webserver->m_table_infos_capacity)
  {
    webserver->m_table_infos_capacity += num_tables;
    if(webserver->m_table_infos != nullptr)
    {
      delete [] webserver->m_table_infos;
    }
    webserver->m_table_infos = new TableInfo[webserver->m_table_infos_capacity];
  }
  uint32_t num_infos = webserver->p_database->meta_data(webserver->m_table_infos, 
                                                        webserver->m_table_infos_capacity);
  // generate json here
  char headers[] = "HTTP/1.1 200 OK\r\nServer: CPi\r\nContent-type: application/json\r\n\r\n";
  
  str_builder_clear(&webserver->m_builder);
  str_builder_append(&webserver->m_builder,
                     "%s { \"tables\" : [", 
                     headers);

  uint32_t i = 0;
  for(; i < num_infos; ++i)
  {

    str_builder_append(&webserver->m_builder, 
                       "{\"name\" : \"%s\",\"size\" : \"%u\"},",
                       webserver->m_table_infos[i].m_name, 
                       webserver->m_table_infos[i].m_size);
  }

  if (i > 0)
  {
    webserver->m_builder.m_pos-=1; // removing the trailing comma
  }
  str_builder_append(&webserver->m_builder, "]}");
}

void
server_thread_handler(WebServer* webserver, 
                      const char* address,
                      const char* port)
{

  addrinfo hints, *server;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, port, &hints, &server);


  int socket_fd = socket( server->ai_family,
                          server->ai_socktype,
                          server->ai_protocol);
  int iSetOption = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,sizeof(iSetOption));

  fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK);

  bind(socket_fd,
       server->ai_addr,
       server->ai_addrlen);

  listen(socket_fd, 10);


  while(keep_running)
  {
    sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd = accept(socket_fd,
                           (struct sockaddr *) &client_addr,
                           &addr_size);
    if(client_fd > 0)
    {
      char buffer[2048];
      memset(buffer,0,sizeof(char)*2048);
      read(client_fd, buffer, 2048);
      printf("%s", buffer);
      fflush(stdout);
      generate_json(webserver);
      write(client_fd, webserver->m_builder.p_buffer, strlen(webserver->m_builder.p_buffer));
      shutdown (client_fd, SHUT_RDWR);      
      close(client_fd);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  close(socket_fd);
  freeaddrinfo(server);
}

WebServer::WebServer() : 
p_database(nullptr),
p_thread(nullptr),
m_table_infos_capacity(0),
m_table_infos(nullptr)
{
  m_builder = str_builder_create();
}

WebServer::~WebServer()
{
  if(p_thread != nullptr)
  {
    stop(); 
  }
  str_builder_destroy(&m_builder);
}

void
WebServer::start(Database* database,
                 const char* address,
                 const char* port)
{
  p_database = database;
  p_thread = new std::thread(server_thread_handler, 
                             this,
                             address,
                             port);
}

void
WebServer::stop()
{
  keep_running = false;
  p_thread->join();
  delete p_thread;
  p_thread = nullptr;
  if(m_table_infos != nullptr)
  {
    delete [] m_table_infos;
    m_table_infos = nullptr;
    m_table_infos_capacity = 0;
  }
}

} /* furious
 */ 
