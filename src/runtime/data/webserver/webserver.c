

// required for getaddrinfo and others
#define _POSIX_C_SOURCE 200112L

#include "../database.h"
#include "webserver.h"
#include "../../../common/memory/numa_alloc.h"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

bool
generate_json_from_infos(fdb_webserver_t* webserver, uint32_t num_infos)
{
  return true;
}

void
generate_json(fdb_webserver_t* webserver)
{
  uint32_t num_tables = fdb_database_num_tables(webserver->p_database);
  if(num_tables > webserver->m_table_infos_capacity)
  {
    webserver->m_table_infos_capacity += num_tables;
    if(webserver->m_table_infos != NULL)
    {
      fdb_numa_free(NULL,webserver->m_table_infos);
    }
    webserver->m_table_infos = (fdb_table_info_t*)fdb_numa_alloc(NULL, 
                                                                 64, 
                                                                 sizeof(fdb_table_info_t)*webserver->m_table_infos_capacity, 
                                                                 FDB_NO_HINT);
  }
  uint32_t num_infos = fdb_database_metadata(webserver->p_database, 
                                         webserver->m_table_infos, 
                                         webserver->m_table_infos_capacity);
  // generate json here
  char headers[] = "HTTP/1.1 200 OK\r\nServer: CPi\r\nContent-type: application/json\r\n\r\n";
  
  fdb_str_builder_clear(&webserver->m_builder);
  fdb_str_builder_append(&webserver->m_builder,
                     "%s { \"tables\" : [", 
                     headers);

  uint32_t i = 0;
  for(; i < num_infos; ++i)
  {

    fdb_str_builder_append(&webserver->m_builder, 
                       "{\"name\" : \"%s\",\"size\" : \"%u\"},",
                       webserver->m_table_infos[i].m_name, 
                       webserver->m_table_infos[i].m_size);
  }

  if (i > 0)
  {
    webserver->m_builder.m_pos-=1; // removing the trailing comma
  }
  fdb_str_builder_append(&webserver->m_builder, "]}");
}

void*
server_thread_handler(void* param)
{
  fdb_webserver_t* webserver = (fdb_webserver_t*)param;

  struct addrinfo hints, *server;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, webserver->m_port, &hints, &server);


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


  while(webserver->m_running)
  {
    struct sockaddr_storage client_addr;
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
    struct timespec reqtime;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = 100000000;
    nanosleep(&reqtime, NULL);
  }
  close(socket_fd);
  freeaddrinfo(server);
  return NULL;
}


void
fdb_webserver_init(fdb_webserver_t* ws)
{
  memset(ws, 0, sizeof(fdb_webserver_t));
  ws->p_database = NULL;
  ws->m_table_infos_capacity = 0;
  ws->m_table_infos = NULL;
  ws->m_running = false;
  fdb_str_builder_init(&ws->m_builder);
}

void
fdb_webserver_release(fdb_webserver_t* ws)
{
  fdb_webserver_stop(ws); 
  fdb_str_builder_release(&ws->m_builder);
}

void
fdb_webserver_start(fdb_webserver_t* ws, 
                    fdb_database_t* db,
                    const char* address,
                    const char* port)
{
  ws->p_database = db;
  ws->m_running = true;
  FDB_COPY_AND_CHECK_STR(ws->m_address, address, FDB_MAX_WEBSERVER_ADDRESS_SIZE);
  FDB_COPY_AND_CHECK_STR(ws->m_port, port, FDB_MAX_WEBSERVER_PORT_SIZE);
  
  pthread_create(&ws->m_thread, 
                 NULL,
                 server_thread_handler, 
                 ws);
}

void
fdb_webserver_stop(fdb_webserver_t* ws)
{
  if(ws->m_running == true)
  {
    ws->m_running = false;
    void* retval;
    pthread_join(ws->m_thread, &retval);
  }

  if(ws->m_table_infos != NULL)
  {
    fdb_numa_free(NULL, ws->m_table_infos);
    ws->m_table_infos = NULL;
    ws->m_table_infos_capacity = 0;
  }
}

