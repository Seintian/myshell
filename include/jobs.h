#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

// Job control API

typedef struct job job_t;

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} job_status_t;

// Job control functions
job_t *job_create(pid_t pgid, const char *command);
void job_set_status(job_t *job, job_status_t status);
void job_list(void);
job_t *job_find(int job_id);
void job_fg(job_t *job);
void job_bg(job_t *job);
void job_cleanup(void);

#endif // JOBS_H
