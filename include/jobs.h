/**
 * @file jobs.h
 * @brief Simple job control (foreground/background) interface.
 */
#ifndef JOBS_H
#define JOBS_H
/** \defgroup group_jobs jobs
 *  @brief Foreground/background job control.
 *  @{ */

#include <sys/types.h>

typedef struct job job_t; /**< Opaque job record. */

/** States a job can be in. */
typedef enum { JOB_RUNNING, /**< Currently running. */
               JOB_STOPPED, /**< Stopped by signal. */
               JOB_DONE }   /**< Finished. */
job_status_t;

// Job control functions
/** Create a new job record and add it to the list. */
job_t *job_create(pid_t pgid, const char *command);
/** Update a job's status in-place. */
void job_set_status(job_t *job, job_status_t status);
/** Print the job list to stdout with IDs and status. */
void job_list(void);
/** Find a job by its ID or return NULL. */
job_t *job_find(int job_id);
/** Bring a stopped job to the foreground; may block waiting. */
void job_fg(job_t *job);
/** Continue a stopped job in the background. */
void job_bg(job_t *job);
/** Remove completed jobs and free their memory. */
void job_cleanup(void);

// Signal and background reaping support
/** Notify jobs module that SIGCHLD occurred (from signal handler). */
void jobs_notify_sigchld(void);
/** Reap finished/stopped children that belong to background jobs. */
void jobs_reap_background(void);

/** @} */

#endif // JOBS_H
