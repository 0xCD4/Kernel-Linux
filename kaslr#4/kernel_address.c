#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

#define KALLSYMS_PATH "/proc/kallsyms"
#define MAX_LINE_LENGTH 256

unsigned long get_kernel_base_offset(void) {
    FILE *file = fopen(KALLSYMS_PATH, "r");
    if (!file) {
        perror("Failed to open /proc/kallsyms");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    unsigned long kaslr_base = 0;
    unsigned long default_base = 0;

    struct utsname uname_data;
    if (uname(&uname_data) != 0) {
        perror("uname failed");
        fclose(file);
        return 0;
    }
    if (strstr(uname_data.machine, "x86_64")) {
        default_base = 0xffffffff81000000;
    } else if (strstr(uname_data.machine, "aarch64")) {
        default_base = 0xffffff8000000000;
    } else {
        printf("Unsupported architecture: %s\n", uname_data.machine);
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        unsigned long addr;
        char type;
        char symbol[MAX_LINE_LENGTH];

        if (sscanf(line, "%lx %c %s", &addr, &type, symbol) == 3) {
            if (strcmp(symbol, "_stext") == 0) {
                kaslr_base = addr;
                break;
            }
        }
    }

    fclose(file);

    if (kaslr_base == 0) {
        printf("Could not find _stext symbol\n");
        return 0;
    }

    return kaslr_base - default_base;
}

int main() {
    if (geteuid() != 0) {
        fprintf(stderr, "This program must be run as root\n");
        return 1;
    }

    unsigned long offset = get_kernel_base_offset();
    if (offset) {
        printf("KASLR Offset: 0x%lx\n", offset);
    }

    return 0;
}
