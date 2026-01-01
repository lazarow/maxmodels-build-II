#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include <stdexcept>

#include "process.hpp"

extern char **environ;

ExecResult run_solver(
    const std::string &solver_path,
    const std::vector<std::string> &args,
    const std::string &stdin_data)
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

    const char *p = stdin_data.data();
    size_t remaining = stdin_data.size();
    while (remaining > 0)
    {
        ssize_t n = write(in_pipe[1], p, remaining);
        if (n > 0)
        {
            p += n;
            remaining -= n;
        }
        else if (errno != EINTR)
        {
            break;
        }
    }
    close(in_pipe[1]);

    string stdout_buf;
    char buf[8192];
    for (;;)
    {
        ssize_t n = read(out_pipe[0], buf, sizeof(buf));
        if (n > 0)
        {
            stdout_buf.append(buf, n);
        }
        else
        {
            break;
        }
    }
    close(out_pipe[0]);

    int status;
    waitpid(pid, &status, 0);

    int exit_code =
        WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    return {exit_code, move(stdout_buf)};
}