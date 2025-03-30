#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <string>
#include <vector>
#include <sys/types.h>

enum JobStatus {
    RUNNING,
    STOPPED,
    DONE
};

struct Job {
    int jobId;
    pid_t pid;
    std::string command;
    JobStatus status;
};

namespace JobControl {
    // Adds a new background job.
    void addJob(pid_t pid, const std::string &command);
    
    // Removes a job from the list.
    void removeJob(pid_t pid);
    
    // Updates the status of a job.
    void updateJobStatus(pid_t pid, JobStatus status);
    
    // Mark a job as done.
    void markJobDone(pid_t pid);
    
    // Lists all current jobs.
    void listJobs();
    
    // Finds a job by its job ID.
    Job* findJob(int jobId);

    // Setup signal handlers (e.g., SIGCHLD) to automatically update job statuses.
    void setupSignalHandlers();
}

#endif // JOB_CONTROL_H
