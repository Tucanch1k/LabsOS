#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

static void my_atexit_handler(void)
{
    printf("[atexit handler] Process PID=%d: atexit handler executed.\n", (int)getpid());
    fflush(stdout);
}

static void sigint_handler(int signo)
{
    printf("\n[SIGINT handler] PID=%d received signal %d (%s)\n", (int)getpid(), signo, strsignal(signo));
    fflush(stdout);
}

static void sigterm_handler(int signo, siginfo_t *info, void *context)
{
    (void)context;

    if (info) {
        printf("\n[SIGTERM handler] PID=%d received signal %d (%s). "
               "Sender PID=%d UID=%d\n",
               (int)getpid(), signo, strsignal(signo),
               info->si_pid, info->si_uid);
    } else {
        printf("\n[SIGTERM handler] PID=%d received signal %d (%s).\n",
               (int)getpid(), signo, strsignal(signo));
    }
    fflush(stdout);
}

int main(void)
{
    pid_t pid;
    int status;

    if (atexit(my_atexit_handler) != 0) {
        fprintf(stderr, "Error: cannot register atexit handler\n");
        return EXIT_FAILURE;
    }

   
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal(SIGINT) failed");
        return EXIT_FAILURE;
    }

    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigterm_handler; 
    sa.sa_flags = SA_SIGINFO;          
    sigemptyset(&sa.sa_mask);          

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction(SIGTERM) failed");
        return EXIT_FAILURE;
    }

    printf("Before fork(): PID=%d PPID=%d\n", (int)getpid(), (int)getppid());
    fflush(stdout);

    pid = fork();
    if (pid < 0) {
        perror("fork() failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        
        printf("[child] PID=%d PPID=%d (this is the child)\n", (int)getpid(), (int)getppid());
        fflush(stdout);

        
        printf("[child] Child will sleep 5 seconds. Send signals to test handlers (SIGINT/SIGTERM).\n");
        fflush(stdout);

        sleep(5);

        printf("[child] Child exiting with code 42.\n");
        fflush(stdout);
        _exit(42); 
    } else {
        
        printf("[parent] After fork: parent PID=%d child PID=%d\n", (int)getpid(), (int)pid);
        fflush(stdout);

        
        printf("[parent] Waiting for child (PID=%d) to finish...\n", (int)pid);
        fflush(stdout);

        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            perror("waitpid failed");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("[parent] Child (PID=%d) exited normally with code %d.\n", (int)pid, exit_status);
        } else if (WIFSIGNALED(status)) {
            int term_sig = WTERMSIG(status);
            printf("[parent] Child (PID=%d) was terminated by signal %d (%s).\n",
                   (int)pid, term_sig, strsignal(term_sig));
        } else {
            printf("[parent] Child (PID=%d) ended with status 0x%x (non-standard).\n", (int)pid, status);
        }
        fflush(stdout);

        printf("[parent] Parent exiting now.\n");
        fflush(stdout);

        exit(EXIT_SUCCESS);
    }

     exit(EXIT_SUCCESS);
}
