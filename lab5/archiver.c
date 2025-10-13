#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <errno.h>

struct file_header {
    char filename[256];
    mode_t mode;
    off_t size;
    time_t mtime;
};

void add_file(const char *archive, const char *filename) {
    int arch_fd = open(archive, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (arch_fd < 0) { perror("open archive"); exit(1); }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) { perror("open file"); close(arch_fd); exit(1); }

    struct stat st;
    fstat(file_fd, &st);

    struct file_header hdr = {0};
    strncpy(hdr.filename, filename, sizeof(hdr.filename) - 1);
    hdr.mode = st.st_mode;
    hdr.size = st.st_size;
    hdr.mtime = st.st_mtime;

    write(arch_fd, &hdr, sizeof(hdr));

    char buf[4096];
    ssize_t bytes;
    while ((bytes = read(file_fd, buf, sizeof(buf))) > 0) {
        write(arch_fd, buf, bytes);
    }

    close(file_fd);
    close(arch_fd);
    printf("Added: %s\n", filename);
}

void show_stat(const char *archive) {
    int arch_fd = open(archive, O_RDONLY);
    if (arch_fd < 0) { perror("open archive"); exit(1); }

    struct file_header hdr;
    int empty = 1;

    printf("Contents of %s:\n", archive);

    while (read(arch_fd, &hdr, sizeof(hdr)) == sizeof(hdr)) {
        empty = 0;
        printf("%s\t%ld bytes\t%s", hdr.filename, (long)hdr.size, ctime(&hdr.mtime));
        lseek(arch_fd, hdr.size, SEEK_CUR);
    }

    if (empty) {
        printf("The archive is empty.\n");
    }

    close(arch_fd);
}

void extract_and_remove(const char *archive, const char *filename) {
    int arch_fd = open(archive, O_RDONLY);
    if (arch_fd < 0) { perror("open archive"); exit(1); }

    int temp_fd = open("temp.arc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) { perror("create temp"); close(arch_fd); exit(1); }

    struct file_header hdr;
    int found = 0;
    char buf[4096];

    while (1) {
        ssize_t hdr_bytes = read(arch_fd, &hdr, sizeof(hdr));
        if (hdr_bytes == 0) break;          // конец архива
        if (hdr_bytes != sizeof(hdr)) break; // ошибка чтения

        if (strcmp(hdr.filename, filename) == 0) {
            found = 1;

            int out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, hdr.mode);
            if (out_fd < 0) { perror("extract"); exit(1); }

            off_t remaining = hdr.size;
            while (remaining > 0) {
                ssize_t chunk = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
                ssize_t bytes = read(arch_fd, buf, chunk);
                if (bytes <= 0) break;
                write(out_fd, buf, bytes);
                remaining -= bytes;
            }
            close(out_fd);

            struct utimbuf times = { hdr.mtime, hdr.mtime };
            utime(filename, &times);

            printf("Extracted and removed: %s\n", filename);
        } else {
            write(temp_fd, &hdr, sizeof(hdr));

            off_t remaining = hdr.size;
            while (remaining > 0) {
                ssize_t chunk = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
                ssize_t bytes = read(arch_fd, buf, chunk);
                if (bytes <= 0) break;
                write(temp_fd, buf, bytes);
                remaining -= bytes;
            }
        }

        if (found && strcmp(hdr.filename, filename) == 0) {
            continue;
        }
    }

    close(arch_fd);
    close(temp_fd);

    if (!found) {
        printf("File not found in archive: %s\n", filename);
        unlink("temp.arc");
    } else {
        if (rename("temp.arc", archive) != 0) {
            perror("rename");
        }
    }
}

// --- справка ---
void help() {
    printf("Usage:\n");
    printf("  ./archiver <archive> -i <file>   Add file to archive\n");
    printf("  ./archiver <archive> -e <file>   Extract and remove file from archive\n");
    printf("  ./archiver <archive> -s          Show archive contents\n");
    printf("  ./archiver -h                    Show help\n");
}


int main(int argc, char *argv[]) {
    if (argc < 2) { help(); return 0; }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        help();
        return 0;
    }

    if (argc < 3) { help(); return 0; }

    const char *archive = argv[1];

    if (strcmp(argv[2], "-i") == 0 || strcmp(argv[2], "--input") == 0) {
        if (argc < 4) { fprintf(stderr, "No file specified\n"); return 1; }
        add_file(archive, argv[3]);
    } else if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--extract") == 0) {
        if (argc < 4) { fprintf(stderr, "No file specified\n"); return 1; }
        extract_and_remove(archive, argv[3]);
    } else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--stat") == 0) {
        show_stat(archive);
    } else {
        help();
    }

    return 0;
}
