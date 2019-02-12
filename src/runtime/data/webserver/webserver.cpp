


#include "webserver.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <thread>


namespace furious
{

bool keep_running = true;

void
server_thread_handler(Database* database,
                      const std::string& address,
                      const std::string& port)
{

  addrinfo hints, *server;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, port.c_str(), &hints, &server);


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

  char headers[] = "HTTP/1.1 200 OK\r\nServer: CPi\r\nContent-type: application/json\r\n\r\n";

  char html[] = "{\"name\" : \"Arnau\"}\r\n";
  char data[2048] = {0};
  snprintf(data, sizeof data, "%s %s", headers, html);


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
      read(client_fd, buffer, 2048);
      printf("%s", buffer);
      fflush(stdout);
      write(client_fd, data, strlen(data));
      shutdown (client_fd, SHUT_RDWR);      
      close(client_fd);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  close(socket_fd);
  freeaddrinfo(server);
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
