
#include "thread.h"

#ifdef FDB_OS_LINUX
#include <sched.h>
void* pthread_handler(void* arg)
{
  struct fdb_thread_task_t* task = (struct fdb_thread_task_t*) arg;
  task->p_fp(task->p_args);
  return NULL;
}
#endif



/**
 * \brief Starts a thread with the given task
 *
 * \param thread The thread to start
 * \param task  The task to run with the thread
 */
void
fdb_thread_start(struct fdb_thread_t* thread, 
                struct fdb_thread_task_t* task)
{
#ifdef FDB_OS_LINUX
  pthread_create(&thread->m_pthread, 
                 NULL, 
                 pthread_handler, 
                 task);
#endif
}

/**
 * \brief Joins a thread
 *
 * \param thread The thread to join
 */
void
fdb_thread_join(struct fdb_thread_t* thread)
{
#ifdef FDB_OS_LINUX
  void* ptr;
  pthread_join(thread->m_pthread, &ptr);
#endif
}

/**
 * \brief Kills a thread
 *
 * \param thread The thread to kill
 */
void
fdb_thread_kill(struct fdb_thread_t* thread)
{
#ifdef FDB_OS_LINUX
  pthread_cancel(thread->m_pthread);
#endif
}

bool
fdb_thread_set_affinity(struct fdb_thread_t* thread, 
                       uint32_t cpuid)
{
#ifdef FDB_OS_LINUX
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpuid, &cpuset);
  uint32_t res = pthread_setaffinity_np(thread->m_pthread,
                                        sizeof(cpu_set_t), 
                                        &cpuset);
  return res == 0;
#endif
}

bool
fdb_thread_set_main_affinity(uint32_t cpuid)
{
#ifdef FDB_OS_LINUX
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpuid, &cpuset);
  uint32_t res = sched_setaffinity(0, sizeof(cpuset), &cpuset);
  return res == 0;
#endif
}
