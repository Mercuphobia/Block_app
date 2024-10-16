#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "log.h"
#include "parsers_data.h"

#define IPSET_LIST_NO_STDOUT "/userfs/bin/ipset list %s > /dev/null 2>&1"
#define IPSET_CREATE "/userfs/bin/ipset create %s hash:ip"
#define IPSET_ADD "/userfs/bin/ipset add %s %s"
#define IPSET_DELETE_RULE "/userfs/bin/ipset destroy %s"

#define IP_TABLES_ADD_INPUT "iptables -I INPUT -m set --match-set %s src -j DROP"
#define IP_TABLES_ADD_OUTPUT "iptables -I OUTPUT -m set --match-set %s src -j DROP"
#define IP_TABLES_ADD_FORWARD "iptables -I FORWARD -m set --match-set %s src -j DROP"

#define IP_TABLES_DELETE_INPUT "iptables -D INPUT -m set --match-set %s src -j DROP"
#define IP_TABLES_DELETE_OUTPUT "iptables -D OUTPUT -m set --match-set %s src -j DROP"
#define IP_TABLES_DELETE_FORWARD "iptables -D FORWARD -m set --match-set %s src -j DROP"

#define IP_TXT_PATH "./data/ip.txt"
#define CHECK_TXT_PATH "./data/check.txt"

#define REST_TIME_BETWEEN_RUN 30

int num_struct = 0;
char command[256];

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
    snprintf(command, sizeof(command), IPSET_LIST_NO_STDOUT, ipset_name);
    int result = system(command);
    return result == 0;
}

void create_ipset(const char* ipset_name) {
    if (!ipset_exists(ipset_name)) {
        snprintf(command, sizeof(command), IPSET_CREATE, ipset_name);
        system(command);
    }
}

void add_ip_to_ipset(const char* ipset_name, const char* ip) {
    snprintf(command, sizeof(command), IPSET_ADD, ipset_name, ip);
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
    web_block_info *list = read_web_block_info(IP_TXT_PATH, &num_struct);
    FILE *check_file = fopen(CHECK_TXT_PATH, "a+");
    if (check_file == NULL) {
        printf("Unable to open file\n");
        return;
    }
    for (int i = 0; i < num_struct; i++) {
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
    check *list = read_check_list(CHECK_TXT_PATH, &num_struct);
    int *rule_active = malloc(num_struct * sizeof(int));
    memset(rule_active, 0, num_struct * sizeof(int));
    while (1) {
        time_t current_time = time(NULL);
        long local_time = get_current_time_in_seconds();
        //printf("Current time in seconds: %ld\n", ld);
        for (int i = 0; i < num_struct; i++) {
            if (local_time >= list[i].start_time_block && local_time <= list[i].end_time_block) {
                if (!rule_active[i]) {
                    char command[256];
                    snprintf(command, sizeof(command), IP_TABLES_ADD_INPUT, list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), IP_TABLES_ADD_OUTPUT, list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), IP_TABLES_ADD_FORWARD, list[i].url);
                    system(command);
                    rule_active[i] = 1;
                    printf("Added rule to block IP in ipset %s\n", list[i].url);
                }
            } else {
                if (rule_active[i]) {
                    snprintf(command, sizeof(command), IP_TABLES_DELETE_INPUT, list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), IP_TABLES_DELETE_OUTPUT, list[i].url);
                    system(command);
                    snprintf(command, sizeof(command), IP_TABLES_DELETE_FORWARD, list[i].url);
                    system(command);
                    rule_active[i] = 0;
                    printf("Removed rule to unblock IP in ipset %s\n", list[i].url);
                }
            }
        }
        sleep(REST_TIME_BETWEEN_RUN);
    }
    free(rule_active);
    free(list);
}

void delete__iptable_rules_chain_and_ipset(){
    check *list = read_check_list(CHECK_TXT_PATH, &num_struct);
    for(int i=0;i<num_struct;i++){
        if(ipset_exists(list[i].url)){
            snprintf(command, sizeof(command), IP_TABLES_DELETE_INPUT, list[i].url);
            system(command);
            snprintf(command, sizeof(command), IP_TABLES_DELETE_OUTPUT, list[i].url);
            system(command);
            snprintf(command, sizeof(command), IP_TABLES_DELETE_FORWARD, list[i].url);
            system(command);
            snprintf(command, sizeof(command), IPSET_DELETE_RULE, list[i].url);
            system(command);
        }
        else{
            printf("ipset %s does not exist.\n", list[i].url);
        }
    }
}
