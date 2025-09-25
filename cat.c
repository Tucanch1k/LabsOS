#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>   

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-n] [-b] [-E] [FILE...]\n", prog);
}

static void process_stream(FILE *f, const char *fname, bool number_all, bool number_nonblank, bool show_ends) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long lineno = 1;

    while ((read = getline(&line, &len, f)) != -1) {
        bool has_newline = (read > 0 && line[read - 1] == '\n');
        bool is_blank = (read == 1 && line[0] == '\n') || (read == 0);

        if (number_all) {
            printf("%6lu\t", lineno++);
        } else if (number_nonblank && !is_blank) {
            printf("%6lu\t", lineno++);
        }

        if (has_newline) {
            if (read > 1) fwrite(line, 1, (size_t)read - 1, stdout);
            if (show_ends) putchar('$');
            putchar('\n');
        } else {
            if (read > 0) fwrite(line, 1, (size_t)read, stdout);
            if (show_ends) putchar('$');
        }
    }
    free(line);
}

int main(int argc, char **argv) {
    bool number_all = false;
    bool number_nonblank = false;
    bool show_ends = false;

    int opt;
    while ((opt = getopt(argc, argv, "nbE")) != -1) {
        switch (opt) {
            case 'n': number_all = true; break;
            case 'b': number_nonblank = true; break;
            case 'E': show_ends = true; break;
            default:
                usage(argv[0]);
                return 2;
        }
    }

    if (number_nonblank) number_all = false; // -b перекрывает -n

    if (optind == argc) {
        // Нет файлов ==> читаем stdin
        process_stream(stdin, NULL, number_all, number_nonblank, show_ends);
    } else {
        // Обрабатываем файлы по очереди
        for (int i = optind; i < argc; i++) {
            const char *fname = argv[i];
            FILE *f = NULL;
            if (strcmp(fname, "-") == 0) {
                f = stdin;
            } else {
                f = fopen(fname, "r");
                if (!f) {
                    fprintf(stderr, "%s: %s: %s\n", argv[0], fname, strerror(errno));
                    continue;
                }
            }
            process_stream(f, fname, number_all, number_nonblank, show_ends);
            if (f != stdin) fclose(f);
        }
    }

    return 0;
}
