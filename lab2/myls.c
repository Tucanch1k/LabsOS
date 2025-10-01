#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <getopt.h>
#include <limits.h>

/* Цвета */
#define COLOR_DIR   "\033[34m"   // синий
#define COLOR_EXEC  "\033[32m"   // зелёный
#define COLOR_LINK  "\033[36m"   // бирюзовый
#define COLOR_RESET "\033[0m"

int opt_long = 0;
int opt_all  = 0;

/* преобразование прав доступа */
void mode_to_str(mode_t mode, char *out) {
    out[0] = S_ISDIR(mode) ? 'd' :
             S_ISLNK(mode) ? 'l' : '-';

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';

    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';

    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';

    out[10] = '\0';
}

/* форматирование времени */
void time_to_str(time_t t, char *buf, size_t size) {
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%b %e %H:%M", tm);
}

/* какой цвет выбрать */
const char *file_color(struct stat *st) {
    if (S_ISLNK(st->st_mode)) return COLOR_LINK;
    if (S_ISDIR(st->st_mode)) return COLOR_DIR;
    if (st->st_mode & S_IXUSR) return COLOR_EXEC;
    return NULL;
}

/* сортировка имён */
int cmp_strcoll(const void *a, const void *b) {
    const char *sa = *(const char * const *)a;
    const char *sb = *(const char * const *)b;
    return strcoll(sa, sb);
}

/* вывод одного файла */
void print_entry(const char *path, const char *name) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("stat");
        return;
    }

    if (!opt_long) {
        const char *c = file_color(&st);
        if (c) printf("%s%s%s", c, name, COLOR_RESET);
        else   printf("%s", name);
        printf("  ");
    } else {
        char perms[11], timebuf[64];
        mode_to_str(st.st_mode, perms);
        time_to_str(st.st_mtime, timebuf, sizeof(timebuf));

        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);

        printf("%s %3ld %s %s %8ld %s ",
            perms,
            (long)st.st_nlink,
            pw ? pw->pw_name : "?",
            gr ? gr->gr_name : "?",
            (long)st.st_size,
            timebuf);

        const char *c = file_color(&st);
        if (c) printf("%s%s%s", c, name, COLOR_RESET);
        else   printf("%s", name);

        if (S_ISLNK(st.st_mode)) {
            char target[PATH_MAX];
            ssize_t len = readlink(path, target, sizeof(target)-1);
            if (len != -1) {
                target[len] = '\0';
                printf(" -> %s", target);
            }
        }
        printf("\n");
    }
}

/* листинг директории */
void list_dir(const char *dirname) {
    DIR *d = opendir(dirname);
    if (!d) {
        perror(dirname);
        return;
    }

    struct dirent *de;
    char **names = NULL;
    size_t count = 0;

    while ((de = readdir(d))) {
        if (!opt_all && de->d_name[0] == '.')
            continue;
        names = realloc(names, (count+1) * sizeof(char*));
        names[count++] = strdup(de->d_name);
    }
    closedir(d);

    qsort(names, count, sizeof(char*), cmp_strcoll);

    if (!opt_long) {
        for (size_t i=0; i<count; i++) {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dirname, names[i]);
            print_entry(path, names[i]);
            free(names[i]);
        }
        printf("\n");
    } else {
        for (size_t i=0; i<count; i++) {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dirname, names[i]);
            print_entry(path, names[i]);
            free(names[i]);
        }
    }
    free(names);
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    int opt;
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch(opt) {
            case 'l': opt_long = 1; break;
            case 'a': opt_all  = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-a] [files...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind == argc) {
        list_dir(".");
    } else {
        for (int i=optind; i<argc; i++) {
            struct stat st;
            if (lstat(argv[i], &st) == -1) {
                perror(argv[i]);
                continue;
            }
            if (S_ISDIR(st.st_mode)) {
                printf("%s:\n", argv[i]);
                list_dir(argv[i]);
            } else {
                print_entry(argv[i], argv[i]);
            }
        }
    }
    return 0;
}
