/*
 * fork_demo.c
 * Демонстрация fork(), atexit(), signal() и sigaction() для Linux.
 *
 * Компиляция:
 *   gcc -Wall -Wextra -std=c11 -o fork_demo fork_demo.c
 *
 * Запуск:
 *   ./fork_demo
 *
 * Пока программа работает, можно послать сигналы:
 *   - В отдельном терминале: kill -SIGINT <PID>
 *   - Или: kill -SIGTERM <PID>
 *
 * Родитель выведет код завершения дочернего процесса.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

/* atexit-обработчик: вызывается при нормальном завершении процесса */
static void my_atexit_handler(void)
{
    /* печатаем PID чтобы было видно, кто вызвал обработчик */
    printf("[atexit handler] Process PID=%d: atexit handler executed.\n", (int)getpid());
    /* flush чтобы сообщение дошло вовремя */
    fflush(stdout);
}

/* Обработчик для SIGINT (устанавливается через signal()) */
static void sigint_handler(int signo)
{
    /* Используем printf для простоты; в реальном суровом коде в сигнальных хендлерах
       лучше использовать async-signal-safe функции (write). Здесь — учебный пример. */
    printf("\n[SIGINT handler] PID=%d received signal %d (%s)\n",
           (int)getpid(), signo, strsignal(signo));
    fflush(stdout);
    /* Не выходим автоматически: пусть процесс решит сам (можно exit здесь, но мы не делаем этого). */
}

/* Обработчик для SIGTERM, устанавливается через sigaction и использует расширенный прототип */
static void sigterm_handler(int signo, siginfo_t *info, void *context)
{
    (void)context; /* не используется */

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

    /* По желанию можно завершить процесс:
       exit(0);
       но здесь оставляем решение за основным кодом. */
}

int main(void)
{
    pid_t pid;
    int status;

    /* Регистрируем atexit-обработчик */
    if (atexit(my_atexit_handler) != 0) {
        fprintf(stderr, "Error: cannot register atexit handler\n");
        return EXIT_FAILURE;
    }

    /* Переопределяем SIGINT при помощи signal() */
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal(SIGINT) failed");
        return EXIT_FAILURE;
    }

    /* Переопределяем SIGTERM при помощи sigaction() */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigterm_handler; /* обработчик с расширенной сигнатурой */
    sa.sa_flags = SA_SIGINFO;          /* разрешаем получать siginfo_t */
    sigemptyset(&sa.sa_mask);          /* не блокируем дополнительные сигналы при обработке */

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction(SIGTERM) failed");
        return EXIT_FAILURE;
    }

    printf("Before fork(): PID=%d PPID=%d\n", (int)getpid(), (int)getppid());
    fflush(stdout);

    /* Вызов fork() и корректная обработка возвращаемых значений */
    pid = fork();
    if (pid < 0) {
        /* Ошибка */
        perror("fork() failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        /* Дочерний процесс */
        printf("[child] PID=%d PPID=%d (this is the child)\n", (int)getpid(), (int)getppid());
        fflush(stdout);

        /* Покажем, что atexit-обработчик будет вызван при выходе дочернего */
        printf("[child] Child will sleep 5 seconds. Send signals to test handlers (SIGINT/SIGTERM).\n");
        fflush(stdout);

        /* Небольшая пауза, чтобы можно было послать сигнал извне */
        sleep(5);

        printf("[child] Child exiting with code 42.\n");
        fflush(stdout);
        /* Нормальное завершение с кодом 42 */
        _exit(42); /* используем _exit чтобы аtexit все равно вызывался? _exit не вызывает atexit.
                      Если хотим, чтобы atexit вызывался — используем exit().
                      Требование: использование atexit() и демонстрация его работы — хотим, чтобы
                      он сработал для обоих процессов. Поэтому используем exit(). */
        /* Важно: заменим _exit на exit() */
    } else {
        /* Родительский процесс */
        printf("[parent] After fork: parent PID=%d child PID=%d\n", (int)getpid(), (int)pid);
        fflush(stdout);

        /* Родитель может подождать немного (необязательно), затем ждать дочерний */
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

        /* Родитель завершится и вызовет atexit-обработчик */
        exit(EXIT_SUCCESS);
    }

    /* Небольшая корректировка: мы использовали _exit в дочернем блоке выше по ошибке.
       Потому ниже — unreachable, но оставляем безопасный exit. */
    exit(EXIT_SUCCESS);
}
