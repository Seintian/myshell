/**
 * @file jobs.c
 * @brief Minimal job control: list, fg/bg, and cleanup.
 */
#define _GNU_SOURCE
#include "jobs.h"
#include "util.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct job {
    int id;
    pid_t pgid;
    char *command;
    job_status_t status;
    struct job *next;
};

/** Head of the job list. */
static job_t *job_list_head = NULL;
/** Monotonic counter for job IDs. */
static int next_job_id = 1;

job_t *job_create(pid_t pgid, const char *command) {
    job_t *job = malloc_safe(sizeof(job_t));
    job->id = next_job_id++;
    job->pgid = pgid;
    job->command = strdup_safe(command);
    job->status = JOB_RUNNING;
    job->next = job_list_head;
    job_list_head = job;
    return job;
}

void job_set_status(job_t *job, job_status_t status) {
    if (job) {
        job->status = status;
    }
}

void job_list(void) {
    job_t *current = job_list_head;
    while (current) {
        const char *status_str;
        switch (current->status) {
        case JOB_RUNNING:
            status_str = "Running";
            break;
        case JOB_STOPPED:
            status_str = "Stopped";
            break;
        case JOB_DONE:
            status_str = "Done";
            break;
        default:
            status_str = "Unknown";
            break;
        }
        printf("[%d] %s\t%s\n", current->id, status_str, current->command);
        current = current->next;
    }
}

job_t *job_find(int job_id) {
    job_t *current = job_list_head;
    while (current) {
        if (current->id == job_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void job_fg(job_t *job) {
    if (job && job->status == JOB_STOPPED) {
        // Send SIGCONT to the process group
        kill(-job->pgid, SIGCONT);
        job->status = JOB_RUNNING;

        // Wait for the job to finish or stop
        int status;
        waitpid(job->pgid, &status, WUNTRACED);

        if (WIFSTOPPED(status)) {
            job->status = JOB_STOPPED;
        } else {
            job->status = JOB_DONE;
        }
    }
}

void job_bg(job_t *job) {
    if (job && job->status == JOB_STOPPED) {
        // Send SIGCONT to the process group
        kill(-job->pgid, SIGCONT);
        job->status = JOB_RUNNING;
        printf("[%d] %s &\n", job->id, job->command);
    }
}

void job_cleanup(void) {
    job_t *current = job_list_head;
    job_t *prev = NULL;

    while (current) {
        if (current->status == JOB_DONE) {
            if (prev) {
                prev->next = current->next;
            } else {
                job_list_head = current->next;
            }
            job_t *to_free = current;
            current = current->next;
            free(to_free->command);
            free(to_free);
        } else {
            prev = current;
            current = current->next;
        }
    }
}
