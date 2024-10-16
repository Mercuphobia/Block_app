#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "log.h"
#include "parsers_data.h"

int get_day_number(const char* day) {
    if (strcmp(day, "Monday") == 0) return 0;
    if (strcmp(day, "Tuesday") == 0) return 1;
    if (strcmp(day, "Wednesday") == 0) return 2;
    if (strcmp(day, "Thursday") == 0) return 3;
    if (strcmp(day, "Friday") == 0) return 4;
    if (strcmp(day, "Saturday") == 0) return 5;
    if (strcmp(day, "Sunday") == 0) return 6;
    return -1;
}

long convert_to_seconds(const char* day, const char* time) {
    int day_number = get_day_number(day);
    if (day_number == -1) {
        printf("Invalid day: %s\n", day);
        return -1;
    }
    int hours, minutes;
    sscanf(time, "%d:%d", &hours, &minutes);
    long total_seconds = day_number * 86400 + hours * 3600 + minutes * 60;
    return total_seconds;
}

long get_current_time_in_seconds() {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    int day_number = tm_now->tm_wday - 1;
    if (day_number < 0) day_number = 6;
    long total_seconds = day_number * 86400 + tm_now->tm_hour * 3600 + tm_now->tm_min * 60 + tm_now->tm_sec;
    return total_seconds;
}

int ipset_exists(const char* ipset_name) {
    char command[256];
    snprintf(command, sizeof(command), "/userfs/bin/ipset list %s > /dev/null 2>&1", ipset_name);
    int result = system(command);
    return result == 0;
}

void create_ipset(const char* ipset_name) {
    if (!ipset_exists(ipset_name)) {
        char command[256];
        snprintf(command, sizeof(command), "/userfs/bin/ipset create %s hash:ip", ipset_name);
        system(command);
    }
}

void add_ip_to_ipset(const char* ipset_name, const char* ip) {
    char command[256];
    snprintf(command, sizeof(command), "/userfs/bin/ipset add %s %s", ipset_name, ip);
    system(command);
}

void clear_file_to_run(const char *filename) {
    FILE *check_file = fopen(filename, "w");
    if (check_file == NULL) {
        perror("Unable to open file");
        return;
    }
    fclose(check_file);
}

bool is_line_in_file(FILE *file, const char *line) {
    char buffer[256];
    rewind(file);
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (strcmp(buffer, line) == 0) {
            return true;
        }
    }
    return false;
}

void get_list() {
    int num_line = 0;
    web_block_info *list = read_web_block_info("./data/ip.txt", &num_line);
    FILE *check_file = fopen("./data/check.txt", "a+");
    if (check_file == NULL) {
        printf("Unable to open file\n");
        return;
    }
    for (int i = 0; i < num_line; i++) {
        char ipset_name[256];
        snprintf(ipset_name, sizeof(ipset_name), "%s", list[i].url);
        create_ipset(ipset_name);
        add_ip_to_ipset(ipset_name, list[i].ip);
        char line[256];
        snprintf(line, sizeof(line), "%s, %ld, %ld\n",
                 list[i].url,
                 convert_to_seconds(list[i].start_day, list[i].start_time),
                 convert_to_seconds(list[i].end_day, list[i].end_time));
        if (!is_line_in_file(check_file, line)) {
            fprintf(check_file, "%s", line);
        }
    }
    fclose(check_file);
}

void run() {
    int num_struct = 0;
    check *list = read_check_list("./data/check.txt", &num_struct);
    int *rule_active = malloc(num_struct * sizeof(int));
    memset(rule_active, 0, num_struct * sizeof(int));
    while (1) {
        time_t current_time = time(NULL);
        long ld = get_current_time_in_seconds();
        printf("Current time in seconds: %ld\n", ld);
        for (int i = 0; i < num_struct; i++) {
            if (ld >= list[i].start_time_block && ld <= list[i].end_time_block) {
                if (!rule_active[i]) {
                    char command[256];
                    snprintf(command, sizeof(command), "iptables -I INPUT -m set --match-set %s src -j DROP", list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), "iptables -I OUTPUT -m set --match-set %s src -j DROP", list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), "iptables -I FORWARD -m set --match-set %s src -j DROP", list[i].url);
                    system(command);
                    rule_active[i] = 1;
                    printf("Added rule to block IP in ipset %s\n", list[i].url);
                }
            } else {
                if (rule_active[i]) {
                    char command[256];
                    snprintf(command, sizeof(command), "iptables -D INPUT -m set --match-set %s src -j DROP", list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), "iptables -D OUTPUT -m set --match-set %s src -j DROP", list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), "iptables -D FORWARD -m set --match-set %s src -j DROP", list[i].url);
                    system(command);
                    rule_active[i] = 0;
                    printf("Removed rule to unblock IP in ipset %s\n", list[i].url);
                }
            }
        }
        sleep(10);
    }
    free(rule_active);
    free(list);
}

void delete__iptable_rules_chain_and_ipset(){
    char command[256];
    int num_struct = 0;
    check *list = read_check_list("./data/check.txt", &num_struct);
    for(int i=0;i<num_struct;i++){
        if(ipset_exists(list[i].url)){
            snprintf(command, sizeof(command), "iptables -D INPUT -m set --match-set %s src -j DROP", list[i].url);
            system(command);
            snprintf(command, sizeof(command), "iptables -D OUTPUT -m set --match-set %s src -j DROP", list[i].url);
            system(command);
            snprintf(command, sizeof(command), "iptables -D FORWARD -m set --match-set %s src -j DROP", list[i].url);
            system(command);
            snprintf(command, sizeof(command), "/userfs/bin/ipset destroy %s", list[i].url);
            system(command);
        }
        else{
            printf("ipset %s does not exist.\n", list[i].url);
        }
    }
}
