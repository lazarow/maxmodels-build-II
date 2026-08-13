#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <time.h>

#include <chrono>
#include <stdexcept>

#include "process.hpp"

extern char **environ;

using namespace std::chrono;

static int remaining_timeout_ms(steady_clock::time_point deadline, bool unlimited)
{
    if (unlimited)
        return -1;
    auto now = steady_clock::now();
    if (now >= deadline)
        return 0;
    auto ms = duration_cast<milliseconds>(deadline - now).count();
    if (ms > 1000000000)
        return 1000000000;
    return static_cast<int>(ms);
}

static void kill_and_reap(pid_t pid, int &status)
{
    kill(pid, SIGKILL);
    while (waitpid(pid, &status, 0) < 0)
    {
        if (errno != EINTR)
            throw runtime_error("waitpid failed");
    }
}

ExecResult run_solver(
    const std::string &solver_path,
    const std::vector<std::string> &args,
    const std::string &stdin_data,
    int timeout_seconds)
{
    int in_pipe[2];
    int out_pipe[2];

    if (pipe2(in_pipe, O_CLOEXEC) != 0)
        throw runtime_error("pipe2 stdin failed");
    if (pipe2(out_pipe, O_CLOEXEC) != 0)
        throw runtime_error("pipe2 stdout failed");

    pid_t pid = fork();
    if (pid < 0)
        throw runtime_error("fork failed");

    if (pid == 0)
    {
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0)
        {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        close(in_pipe[1]);
        close(out_pipe[0]);
        vector<char *> argv;
        argv.reserve(args.size() + 2);
        argv.push_back(const_cast<char *>(solver_path.c_str()));
        for (const auto &a : args)
            argv.push_back(const_cast<char *>(a.c_str()));
        argv.push_back(nullptr);
        execve(solver_path.c_str(), argv.data(), environ);
        _exit(127);
    }

    close(in_pipe[0]);
    close(out_pipe[1]);
    signal(SIGPIPE, SIG_IGN);

    const bool unlimited = timeout_seconds <= 0;
    const auto deadline = unlimited
                              ? steady_clock::time_point::max()
                              : steady_clock::now() + seconds(timeout_seconds);

    const char *p = stdin_data.data();
    size_t remaining = stdin_data.size();
    bool stdin_open = true;
    if (remaining == 0)
    {
        close(in_pipe[1]);
        stdin_open = false;
    }

    string stdout_buf;
    char buf[8192];
    bool stdout_open = true;
    bool timed_out = false;
    int status = 0;

    while (stdout_open)
    {
        pollfd fds[2];
        nfds_t nfds = 0;
        int out_idx = -1;
        int in_idx = -1;

        fds[nfds].fd = out_pipe[0];
        fds[nfds].events = POLLIN;
        out_idx = static_cast<int>(nfds++);
        if (stdin_open)
        {
            fds[nfds].fd = in_pipe[1];
            fds[nfds].events = POLLOUT;
            in_idx = static_cast<int>(nfds++);
        }

        int pret = poll(fds, nfds, remaining_timeout_ms(deadline, unlimited));
        if (pret < 0)
        {
            if (errno == EINTR)
                continue;
            if (stdin_open)
                close(in_pipe[1]);
            close(out_pipe[0]);
            kill_and_reap(pid, status);
            throw runtime_error("poll failed");
        }
        if (pret == 0)
        {
            timed_out = true;
            break;
        }

        if (stdin_open && (fds[in_idx].revents & (POLLOUT | POLLERR | POLLHUP)))
        {
            if (fds[in_idx].revents & POLLOUT)
            {
                ssize_t n = write(in_pipe[1], p, remaining);
                if (n > 0)
                {
                    p += n;
                    remaining -= n;
                    if (remaining == 0)
                    {
                        close(in_pipe[1]);
                        stdin_open = false;
                    }
                }
                else if (errno != EINTR)
                {
                    close(in_pipe[1]);
                    stdin_open = false;
                }
            }
            else
            {
                close(in_pipe[1]);
                stdin_open = false;
            }
        }

        if (fds[out_idx].revents & (POLLIN | POLLHUP | POLLERR))
        {
            if (fds[out_idx].revents & POLLIN)
            {
                ssize_t n = read(out_pipe[0], buf, sizeof(buf));
                if (n > 0)
                    stdout_buf.append(buf, n);
                else if (n == 0 || errno != EINTR)
                    stdout_open = false;
            }
            else
            {
                stdout_open = false;
            }
        }
    }

    if (stdin_open)
        close(in_pipe[1]);

    if (timed_out)
    {
        close(out_pipe[0]);
        kill_and_reap(pid, status);
        return {-1, move(stdout_buf), true};
    }

    close(out_pipe[0]);

    while (true)
    {
        int flags = unlimited ? 0 : WNOHANG;
        pid_t r = waitpid(pid, &status, flags);
        if (r == pid)
            break;
        if (r < 0)
        {
            if (errno == EINTR)
                continue;
            throw runtime_error("waitpid failed");
        }
        int ms = remaining_timeout_ms(deadline, unlimited);
        if (ms == 0)
        {
            timed_out = true;
            kill_and_reap(pid, status);
            break;
        }
        timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = static_cast<long>((ms < 50 ? ms : 50) * 1000000L);
        nanosleep(&ts, nullptr);
    }

    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return {exit_code, move(stdout_buf), timed_out};
}
