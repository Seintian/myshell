/**
 * @file jobs.c
 * @brief Minimal job control: list, fg/bg, and cleanup.
 */
#include "jobs.h"
#include "shell.h"
#include "util.h"
#include <errno.h>
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
/** Set when SIGCHLD occurs; drained by jobs_reap_background. */
static volatile sig_atomic_t jobs_sigchld_flag = 0;

job_t *job_create(pid_t pgid, const char *command) {
    if (!command)
        return NULL;
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
    if (job)
        job->status = status;
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
        if (current->id == job_id)
            return current;
        current = current->next;
    }
    return NULL;
}

void job_fg(job_t *job) {
    if (!job)
        return;
    if (job->status == JOB_STOPPED) {
        // Send SIGCONT to the process group
        if (kill(-job->pgid, SIGCONT) == -1) {
            perror("kill(SIGCONT)");
            return;
        }
        job->status = JOB_RUNNING;
    }

    // If interactive and attached to a tty, give terminal control to the job's PGID
    int gave_tty = 0;
    if (shell_interactive && isatty(STDIN_FILENO)) {
        if (tcsetpgrp(STDIN_FILENO, job->pgid) == -1) {
            if (errno != ENOTTY && errno != EPERM && errno != EINVAL)
                perror("tcsetpgrp");
        } else {
            gave_tty = 1;
        }
    }

    // Wait for the job to finish or stop (any child in group)
    int status = 0;
    pid_t w = waitpid(-job->pgid, &status, WUNTRACED);
    if (w == -1) {
        if (errno != ECHILD)
            perror("waitpid");
        return;
    }

    if (WIFSTOPPED(status)) {
        job->status = JOB_STOPPED;
    } else {
        job->status = JOB_DONE;
    }

    // Restore terminal control back to the shell's process group
    if (gave_tty) {
        pid_t shell_pgid = getpgrp();
        if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
            if (errno != ENOTTY && errno != EPERM && errno != EINVAL)
                perror("tcsetpgrp");
        }
    }
}

void job_bg(job_t *job) {
    if (!job)
        return;
    if (job->status == JOB_STOPPED) {
        // Send SIGCONT to the process group
        if (kill(-job->pgid, SIGCONT) == -1) {
            perror("kill(SIGCONT)");
            return;
        }
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

void jobs_notify_sigchld(void) {
    jobs_sigchld_flag = 1;
}

static job_t *job_find_by_pgid(pid_t pgid) {
    job_t *cur = job_list_head;
    while (cur) {
        if (cur->pgid == pgid)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void jobs_reap_background(void) {
    if (!jobs_sigchld_flag)
        return;
    jobs_sigchld_flag = 0;

    int status;
    pid_t pid;
    // Reap all available children without blocking
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        pid_t pgid = getpgid(pid);
        if (pgid == -1)
            continue;
        job_t *job = job_find_by_pgid(pgid);
        if (!job)
            continue;
        if (WIFSTOPPED(status))
            job->status = JOB_STOPPED;
        else if (WIFCONTINUED(status))
            job->status = JOB_RUNNING;
        else if (WIFEXITED(status) || WIFSIGNALED(status))
            job->status = JOB_DONE;
    }
}
