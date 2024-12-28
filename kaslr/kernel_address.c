#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

unsigned long long find_kernel_base(void) {
    FILE *fp;
    char line[256];
    unsigned long long addr_min = -1ULL;
    unsigned long long addr;
    char type;
    char symbol[64];

    fp = fopen("/proc/kallsyms", "r");
    if (!fp) {
        perror("Failed to open /proc/kallsyms");
        return 0;
    }

    // Find the lowest kernel text address
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%llx %c %s", &addr, &type, symbol) == 3) {
            // Look for text (T) or text-related (t) symbols
            if (type == 't' || type == 'T') {
                if (addr < addr_min) {
                    addr_min = addr;
                }
            }
        }
    }

    fclose(fp);
    return addr_min;
}

int main() {
    unsigned long long kernel_base = find_kernel_base();
    
    if (kernel_base) {
        printf("Actual Kernel Base Address: 0x%llx\n", kernel_base);
    } else {
        printf("Failed to find kernel base address\n");
    }

    return 0;
}
