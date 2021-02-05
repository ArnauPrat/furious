


#ifndef _FURIOUS_WEBSERVER_H_
#define _FURIOUS_WEBSERVER_H_ value

#include "../../../common/types.h"
#include "../../../common/platform.h"
#include "../../../common/str_builder.h"

#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


struct fdb_webserver_t
{
  struct fdb_database_t*        p_database; 
  pthread_t                     m_thread;
  uint32_t                      m_table_infos_capacity;
  struct fdb_table_info_t*      m_table_infos;
  struct fdb_str_builder_t             m_builder;
  bool                          m_running;
  char                          m_port[FDB_MAX_WEBSERVER_PORT_SIZE];
  char                          m_address[FDB_MAX_WEBSERVER_ADDRESS_SIZE];
}; 


void
fdb_webserver_init(struct fdb_webserver_t* ws);

void
fdb_webserver_release(struct fdb_webserver_t* ws);

void
fdb_webserver_start(struct fdb_webserver_t* ws, 
                    struct fdb_database_t* db, 
                    const char* address,
                    const char* port);

void
fdb_webserver_stop(struct fdb_webserver_t* ws);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FURIOUS_WEBSERVER_H_ */

