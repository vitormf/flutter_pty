#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

pid_t pty_forkpty(
    int *master,
    int *slave,
    const struct termios *termp,
    const struct winsize *winp)
{
    int ptm = open("/dev/ptmx", O_RDWR | O_NOCTTY);

    if (ptm < 0)
    {
        return -1;
    }

    fcntl(ptm, F_SETFD, FD_CLOEXEC);

    if (grantpt(ptm) || unlockpt(ptm))
    {
        return -1;
    }

    char *devname;

    if ((devname = ptsname(ptm)) == NULL)
    {
        return -1;
    }

    int pts = open(devname, O_RDWR | O_NOCTTY);
    if (pts < 0)
    {
        return -1;
    }

    if (termp)
    {
        tcsetattr(pts, TCSAFLUSH, termp);
    }

    if (winp)
    {
        ioctl(pts, TIOCSWINSZ, winp);
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        return -1;
    }

    if (pid == 0)
    {
        // Reset all signal handlers to defaults. The parent may have installed
        // handlers (e.g. Crashlytics) that call async-signal-unsafe functions.
        // Running them in the child side of fork, before exec, causes a crash
        // because macOS file system APIs abort when called post-fork pre-exec.
        for (int sig = 1; sig < NSIG; sig++)
        {
            signal(sig, SIG_DFL);
        }

        setsid();

        // Close the parent-inherited slave PTY (opened with O_NOCTTY) and
        // re-open without O_NOCTTY. On macOS/BSD, a session leader that opens
        // a tty without O_NOCTTY automatically acquires it as the controlling
        // terminal. This avoids TIOCSCTTY, which the macOS App Sandbox denies.
        close(pts);
        pts = open(devname, O_RDWR);
        if (pts < 0)
            _exit(-1);

        dup2(pts, STDIN_FILENO);
        dup2(pts, STDOUT_FILENO);
        dup2(pts, STDERR_FILENO);

        if (pts > 2)
        {
            close(pts);
        }
    }
    else
    {
        *master = ptm;
        if (slave)
        {
            *slave = pts;
        }
    }

    return pid;
}
