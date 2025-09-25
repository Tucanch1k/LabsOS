#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <regex.h>

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s PATTERN [FILE...]\n", prog);
}

static int process_stream(FILE *f, const char *fname, regex_t *re, bool multiple_files) {
    char *line = NULL;
    size_t len = 0;
    ssize_t r;
    int matched = 1; // 0 = нашёл совпадение, 1 = нет
    while ((r = getline(&line, &len, f)) != -1) {
        if (regexec(re, line, 0, NULL, 0) == 0) {
            matched = 0;
            if (multiple_files && fname)
                printf("%s:%s", fname, line);
            else
                fputs(line, stdout);
        }
    }
    free(line);
    return matched;
}

int main(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
            default:
                usage(argv[0]);
                return 2;
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
        return 2;
    }

    const char *pattern = argv[optind++];
    int files_count = argc - optind;
    bool multiple_files = files_count > 1;

    // Компиляция regex
    regex_t re;
    int ret = regcomp(&re, pattern, REG_EXTENDED | REG_NEWLINE);
    if (ret != 0) {
        char errbuf[256];
        regerror(ret, &re, errbuf, sizeof(errbuf));
        fprintf(stderr, "%s: invalid regex '%s': %s\n", argv[0], pattern, errbuf);
        return 2;
    }

    int exit_code = 1; // 0 = совпадения найдены, 1 = нет совпадений, 2 = ошибка
    bool had_error = false;

    if (files_count == 0) {
        int res = process_stream(stdin, NULL, &re, false);
        if (res == 0) exit_code = 0;
    } else {
        for (int i = optind; i < argc; i++) {
            const char *fname = argv[i];
            FILE *f = NULL;
            if (strcmp(fname, "-") == 0) {
                f = stdin;
            } else {
                f = fopen(fname, "r");
                if (!f) {
                    fprintf(stderr, "%s: %s: %s\n", argv[0], fname, strerror(errno));
                    had_error = true;
                    continue;
                }
            }
            int res = process_stream(f, fname, &re, multiple_files);
            if (res == 0) exit_code = 0;
            if (f != stdin) fclose(f);
        }
    }

    regfree(&re);

    if (had_error && exit_code == 1) exit_code = 2;
    return exit_code;
}
