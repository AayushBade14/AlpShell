#include "job_control.h"
#include <iostream>
#include <algorithm>
#include <csignal>
#include <sys/wait.h>
#include <errno.h>

static std::vector<Job> jobs;
static int nextJobId = 1;

namespace JobControl {

    void addJob(pid_t pid, const std::string &command) {
        jobs.push_back({nextJobId++, pid, command, RUNNING});
    }

    void removeJob(pid_t pid) {
        jobs.erase(std::remove_if(jobs.begin(), jobs.end(),
                    [pid](const Job &j) { return j.pid == pid; }),
                    jobs.end());
    }

    void updateJobStatus(pid_t pid, JobStatus status) {
        for (auto &job : jobs) {
            if (job.pid == pid) {
                job.status = status;
                break;
            }
        }
    }

    void markJobDone(pid_t pid) {
        updateJobStatus(pid, DONE);
    }

    void listJobs() {
        for (const auto &job : jobs) {
            std::string statusStr;
            switch (job.status) {
                case RUNNING: statusStr = "Running"; break;
                case STOPPED: statusStr = "Stopped"; break;
                case DONE:    statusStr = "Done";    break;
            }
            std::cout << '[' << job.jobId << "] " << job.pid << " " << statusStr 
                      << " - " << job.command << std::endl;
        }
    }

    Job* findJob(int jobId) {
        for (auto &job : jobs) {
            if (job.jobId == jobId)
                return &job;
        }
        return nullptr;
    }

    // SIGCHLD handler to update job statuses when background processes terminate.
    void sigchldHandler(int /*sig*/) {
        int savedErrno = errno;
        pid_t pid;
        int status;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            markJobDone(pid);
        }
        errno = savedErrno;
    }

    void setupSignalHandlers() {
        struct sigaction sa;
        sa.sa_handler = sigchldHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        sigaction(SIGCHLD, &sa, nullptr);
    }
}
