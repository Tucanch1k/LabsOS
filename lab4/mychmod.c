#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/stat.h>  
#include <errno.h>


int is_octal_number(const char *s) {
    for (size_t i = 0; s[i]; i++) {
        if (s[i] < '0' || s[i] > '7') return 0;   
    }
    return 1;
}


int symbolic_chmod(const char *mode, const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("stat");
        return -1;
    }

    mode_t new_mode = st.st_mode;
    const char *p = mode;

    int who_u = 0, who_g = 0, who_o = 0;
    while (*p == 'u' || *p == 'g' ||  *p == 'o' ||  *p == 'a') {  
        if (*p == 'u') who_u = 1;
        if (*p == 'g') who_g = 1;
        if (*p == 'o') who_o = 1;
        if (*p == 'a') who_u = who_g = who_o = 1;
        p++;
    }

    if (!who_u && !who_g && !who_o) {
        who_u = who_g = who_o = 1; 
    }

    char op = *p++;
    if (op != '+' && op != '-') {
        fprintf(stderr, "Invalid operator in mode: %s\n", mode);
        return -1;
    }

    int add_r = 0, add_w = 0, add_x = 0;
    while (*p) {
        if (*p == 'r') add_r = 1;
        else if (*p == 'w') add_w = 1;
        else if (*p == 'x') add_x = 1;
        else {
            fprintf(stderr, "Invalid permission: %c\n", *p);
            return -1;
        }
        p++;
    }

    mode_t bits = 0;
    if (who_u) {
        if (add_r) bits |= S_IRUSR;
        if (add_w) bits |= S_IWUSR;
        if (add_x) bits |= S_IXUSR;
    }
    if (who_g) {
        if (add_r) bits |= S_IRGRP;
        if (add_w) bits |= S_IWGRP;
        if (add_x) bits |= S_IXGRP;
    }
    if (who_o) {
        if (add_r) bits |= S_IROTH;
        if (add_w) bits |= S_IWOTH;
        if (add_x) bits |= S_IXOTH;
    }

    if (op == '+')
        new_mode |= bits;
    else
        new_mode &= ~bits;

    if (chmod(filename, new_mode) == -1) {
        perror("chmod");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <mode> <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *mode = argv[1];
    const char *filename = argv[2];

    if (is_octal_number(mode)) {
        mode_t perms = strtol(mode, NULL, 8);
        if (chmod(filename, perms) == -1) {
            perror("chmod");
            return EXIT_FAILURE;
        }
    } else {
        if (symbolic_chmod(mode, filename) == -1) {
            return EXIT_FAILURE;
        }
    }

    printf("Permissions for '%s' successfully changed.\n", filename);
    return EXIT_SUCCESS;
}
