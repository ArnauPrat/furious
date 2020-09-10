#ifdef FDB_OS_LINUX
  #ifndef _GNU_SOURCE
  #define _GNU_SOURCE
  #endif
#endif

#ifndef _FDB_THREAD_H_
#define _FDB_THREAD_H_

#include "platform.h"

#ifdef FDB_OS_LINUX
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

typedef void (*fdb_thread_task_function_t)(void* arg);

typedef struct fdb_thread_task_t
{

  fdb_thread_task_function_t p_fp;

  void* p_args;

} fdb_thread_task_t;


typedef struct fdb_thread_t
{
#ifdef FDB_OS_LINUX
  pthread_t m_pthread;
#endif
} fdb_thread_t;


/**
 * \brief Starts a thread with the given task
 *
 * \param thread The thread to start
 * \param task  The task to run with the thread
 */
void
fdb_thread_start(fdb_thread_t* thread, 
                 fdb_thread_task_t* task);

/**
 * \brief Joins a thread
 *
 * \param thread The thread to join
 */
void
fdb_thread_join(fdb_thread_t* thread);

/**
 * \brief Kills a thread
 *
 * \param thread The thread to kill
 */
void
fdb_thread_kill(fdb_thread_t* thread);

/**
 * \brief Sets the affinity for the given thread
 *
 * \param thread The thread to set the affinity for
 * \param cpuid The id of the cpu to set the affinity to
 */
bool
fdb_thread_set_affinity(fdb_thread_t* thread, 
                       uint32_t cpuid);

/**
 * \brief Sets the affinity of the main thread
 *
 * \param cpuid The id of the cpu to set the affinity to
 */
bool
fdb_thread_set_main_affinity(uint32_t cpuid);

#ifdef __cplusplus
}
#endif


#endif /* ifndef _GS_THREAD_H_ */
