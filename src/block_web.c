#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IP_LENGTH 16
#define SET_NAME "blocklist"

void create_ipset() {
    char command[256];

    // Tạo ipset
    snprintf(command, sizeof(command), "ipset create %s hash:ip", SET_NAME);
    int result = system(command);

    if (result == -1) {
        perror("Failed to create ipset");
        exit(EXIT_FAILURE);
    }
}

void block_ips_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char ip[MAX_IP_LENGTH];
    while (fgets(ip, sizeof(ip), file)) {
        ip[strcspn(ip, "\n")] = 0; // Xóa ký tự newline

        char command[256];
        snprintf(command, sizeof(command), "ipset add %s %s", SET_NAME, ip);
        int result = system(command);

        if (result == -1) {
            perror("Failed to run ipset command");
        } else {
            // Thêm quy tắc iptables để chặn địa chỉ IP
            snprintf(command, sizeof(command), "iptables -A INPUT -m set --match-set %s src -j DROP", SET_NAME);
            result = system(command);

            if (result == -1) {
                perror("Failed to run iptables command");
            }
        }
    }
    fclose(file);
}