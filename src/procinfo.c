#include "../include/common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
static void usage(const char *a)
{
    fprintf(stderr, "Usage: %s <pid>\n", a);
    exit(1);
}
static int isnum(const char *s)
{
    for (; *s; s++)
        if (!isdigit(*s))
            return 0;
    return 1;
}
int main(int c, char **v)
{
    if (c != 2 || !isnum(v[1]))
        usage(v[0]);
    char path[256], line[8192], cmd[4096] = {0};
    FILE *f;

    // Read /proc/<pid>/stat
    snprintf(path, sizeof(path), "/proc/%s/stat", v[1]);
    f = fopen(path, "r");
    if (!f)
    {
        if (errno == ENOENT)
            fprintf(stderr, "ERROR: PID not found: %s\n", path);
        else if (errno == EACCES)
            fprintf(stderr, "ERROR: Permission denied: %s\n", path);
        else
            DIE(path);
        exit(EXIT_FAILURE);
    }
    if (!fgets(line, sizeof(line), f))
        DIE_MSG("failed to read stat");
    fclose(f);

    // Parse state, ppid, utime, stime
    char *lp = strchr(line, '('), *rp = strrchr(line, ')');
    if (!lp || !rp || rp < lp)
        DIE_MSG("bad stat format");

    char state;
    int ppid;
    unsigned long long utime, stime;
    if (sscanf(rp + 2, "%c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu",
               &state, &ppid, &utime, &stime) != 4)
        DIE_MSG("parse error");

    long ticks = sysconf(_SC_CLK_TCK);
    if (ticks <= 0)
        ticks = 100;
    double cpu = (double)(utime + stime) / (double)ticks;

    // Read VmRSS from /proc/<pid>/status
    snprintf(path, sizeof(path), "/proc/%s/status", v[1]);
    long rss = 0;
    if ((f = fopen(path, "r")))
    {
        while (fgets(line, sizeof(line), f))
            if (sscanf(line, "VmRSS: %ld", &rss) == 1)
                break;
        fclose(f);
    }

    // Read /proc/<pid>/cmdline
    snprintf(path, sizeof(path), "/proc/%s/cmdline", v[1]);
    if ((f = fopen(path, "rb")))
    {
        size_t n = fread(cmd, 1, sizeof(cmd) - 1, f);
        fclose(f);
        for (size_t i = 0; i < n; i++)
            if (cmd[i] == '\0')
                cmd[i] = ' ';
        while (n > 0 && (cmd[n - 1] == ' ' || cmd[n - 1] == '\n'))
            cmd[--n] = '\0';
    }
    if (!cmd[0])
    {
        size_t len = (size_t)(rp - lp - 1);
        if (len >= sizeof(cmd))
            len = sizeof(cmd) - 1;
        memcpy(cmd, lp + 1, len);
        cmd[len] = '\0';
    }

    printf("PID: %s\nState: %c\nPPID: %d\nCmd: %s\nCPU: %.3f\nVmRSS: %ld\n",
           v[1], state, ppid, cmd, cpu, rss);

    return 0;
}
