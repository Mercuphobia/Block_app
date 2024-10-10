#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "parsers_data.h"

#define IPSET_NAME "blocklist"

char command[MAX_LENGTH];
char line[MAX_LENGTH];
char ip[MAX_LENGTH];
char start_day[MAX_LENGTH];
char start_time[MAX_LENGTH];
char end_day[MAX_LENGTH];
char end_time[MAX_LENGTH];
const char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};


void create_ipset() {
    snprintf(command, sizeof(command), "ipset create %s hash:ip -exist", IPSET_NAME);
    int result = system(command);
    if (result == -1) {
        perror("Failed to create ipset");
        exit(EXIT_FAILURE);
    }
    snprintf(command, sizeof(command), "ipset flush %s", IPSET_NAME);
    result = system(command);
    if (result == -1) {
        perror("Failed to flush ipset");
        exit(EXIT_FAILURE);
    }
}
void block_ips_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return;
    }
    while (fgets(ip, sizeof(ip), file)) {
        ip[strcspn(ip, "\n")] = 0;
        if (strlen(ip) == 0) {
            continue;
        }
        char command[256];
        snprintf(command, sizeof(command), "ipset add %s %s -exist", IPSET_NAME, ip);
        int result = system(command);

        if (result == -1) {
            perror("Failed to run ipset command");
        }
    }
    fclose(file);
    char command[256];
    snprintf(command, sizeof(command), "iptables -A INPUT -m set --match-set %s src -j DROP", IPSET_NAME);
    int result = system(command);
    if (result == -1) {
        perror("Failed to run iptables command");
    }
}

void change_time_to_UTC(char *time_str, char *day_str, int offset) {
    int hours, minutes;
    int day_index = -1;
    for (int i = 0; i < 7; i++) {
        if (strcmp(day_str, days[i]) == 0) {
            day_index = i;
            break;
        }
    }
    sscanf(time_str, "%d:%d", &hours, &minutes);
    hours -= offset;
    if (hours < 0) {
        hours += 24;
        if (day_index != -1) {
            day_index = (day_index - 1 + 7) % 7;
            strcpy(day_str, days[day_index]);
        }
    }
    snprintf(time_str, MAX_LENGTH, "%02d:%02d", hours, minutes);
}

void block_ips_by_time(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return;
    }
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        sscanf(line, "%15[^,],%9[^,],%5[^,],%9[^,],%5[^,]", ip, start_day, start_time, end_day, end_time);
        change_time_to_UTC(start_time, start_day, 7);
        change_time_to_UTC(end_time, end_day, 7);
        if (strcmp(start_day, end_day) == 0) {
            //start_day
            snprintf(command, sizeof(command), 
            "sudo iptables -A INPUT -s %s -m time --weekdays %s --timestart %s --timestop %s -j REJECT --reject-with icmp-port-unreachable",
            ip, start_day, start_time, end_time);
            printf("%s\n", command);
            int result = system(command);
        } else {
            int start_day_index = -1, end_day_index = -1;
            for (int i = 0; i < 7; i++) {
                if (strcmp(start_day, days[i]) == 0) {
                    start_day_index = i;
                }
                if (strcmp(end_day, days[i]) == 0) {
                    end_day_index = i;
                }
            }
            snprintf(command, sizeof(command), 
                "sudo iptables -A INPUT -s %s -m time --weekdays %s --timestart %s --timestop 23:59 -j REJECT --reject-with icmp-port-unreachable",
                ip, start_day, start_time);
            printf("%s\n", command);
            system(command);

            // between_day
            if (start_day_index != -1 && end_day_index != -1) {
                char days_in_between[MAX_LENGTH] = "";
                int next_day = (start_day_index + 1) % 7;
                while (next_day != end_day_index) {
                    if (strlen(days_in_between) > 0) {
                        strcat(days_in_between, ",");
                    }
                    strcat(days_in_between, days[next_day]);
                    next_day = (next_day + 1) % 7;
                }
                int size_of_between_day = (int)strlen(days_in_between);
                if(size_of_between_day != 0){
                    snprintf(command, sizeof(command), 
                        "sudo iptables -A INPUT -s %s -m time --weekdays %s --timestart 00:00 --timestop 23:59 -j REJECT --reject-with icmp-port-unreachable",
                        ip, days_in_between);
                    printf("%s\n", command);
                    system(command);
                }
            }
            // end_day
            snprintf(command, sizeof(command), 
                "sudo iptables -A INPUT -s %s -m time --weekdays %s --timestart 00:00 --timestop %s -j REJECT --reject-with icmp-port-unreachable",
                ip, end_day, end_time);
            printf("%s\n", command);
            system(command);
        }      
    }
    fclose(file);
}