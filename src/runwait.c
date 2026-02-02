#include "../include/common.h"
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static void usage(const char *a)
{
    fprintf(stderr, "Usage: %s <cmd> [args]\n", a);
    exit(1);
}
static double d(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}
int main(int c, char **v)
{
    if (c < 2)
        usage(v[0]);

    struct timespec start, end;

    // Get start time using CLOCK_MONOTONIC
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
        DIE("clock_gettime");

    // Fork the process
    pid_t pid = fork();
    if (pid < 0)
        DIE("fork");

    if (pid == 0)
    {
        // Child process: execute the command
        execvp(v[1], &v[1]);
        // If execvp returns, it failed
        DIE(v[1]);
    }

    // Parent process: wait for child
    int status;
    if (waitpid(pid, &status, 0) == -1)
        DIE("waitpid");

    // Get end time
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
        DIE("clock_gettime");

    // Calculate elapsed time
    double elapsed = d(start, end);

    // Print results
    printf("Child PID: %d\n", pid);

    if (WIFEXITED(status))
    {
        printf("exit=%d\n", WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("signal=%d\n", WTERMSIG(status));
    }

    printf("Elapsed time: %.6f seconds\n", elapsed);

    return 0;
}
