// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "log.h"
// #include "parsers_data.h"


// #define CHAIN_NAME "BLOCK_IP_CHAIN"

// char command[MAX_LENGTH];
// char line[MAX_LENGTH];
// char ip[MAX_LENGTH];
// char start_day[MAX_LENGTH];
// char start_time[MAX_LENGTH];
// char end_day[MAX_LENGTH];
// char end_time[MAX_LENGTH];
// const char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

// void change_time_to_UTC(char *time_str, char *day_str) {
//     int hours, minutes;
//     int day_index = -1;
//     int offset = 7;
//     for (int i = 0; i < 7; i++) {
//         if (strcmp(day_str, days[i]) == 0) {
//             day_index = i;
//             break;
//         }
//     }
//     sscanf(time_str, "%d:%d", &hours, &minutes);
//     hours -= offset;
//     if (hours < 0) {
//         hours += 24;
//         if (day_index != -1) {
//             day_index = (day_index - 1 + 7) % 7;
//             strcpy(day_str, days[day_index]);
//         }
//     }
//     snprintf(time_str, MAX_LENGTH, "%02d:%02d", hours, minutes);
// }

// void create_iptables_chain() {
//     snprintf(command, sizeof(command), "iptables -N BLOCK_IP_CHAIN");
//     system(command);
//     system("iptables -A INPUT -j BLOCK_IP_CHAIN");
// }

// int ipset_exists(const char* ipset_name) {
//     char command[256];
//     snprintf(command, sizeof(command), "ipset list %s > /dev/null 2>&1", ipset_name);
//     int result = system(command);
//     return result == 0;
// }

// void create_ipset(const char* ipset_name) {
//     if (!ipset_exists(ipset_name)) {
//         char command[256];
//         snprintf(command, sizeof(command), "ipset create %s hash:ip", ipset_name);
//         system(command);

//         FILE *file = fopen("./data/ipset.txt", "a");
//         if (file) {
//             fprintf(file, "%s\n", ipset_name);
//             fclose(file);
//         }
//     }
// }

// void add_ip_to_ipset(const char* ipset_name, const char* ip) {
//     char command[256];
//     snprintf(command, sizeof(command), "ipset add %s %s", ipset_name, ip);
//     system(command);
// }

// int check_iptables_rule_exists(const char* ipset_name) {
//     char command[256];
//     snprintf(command, sizeof(command), "iptables -C INPUT -m set --match-set %s src -j DROP > /dev/null 2>&1", ipset_name);
//     return system(command) == 0; 
// }

// void add_iptables_rule(const char* chain_name, const char* ipset_name, char *start_day, char *start_time, char *end_day, char *end_time) {
//     if (strcmp(start_day, end_day) == 0) {
//         snprintf(command, sizeof(command),
//             "iptables -C %s -m time --timestart %s --timestop %s --weekdays %s -m set --match-set %s src -j DROP 2>/dev/null",
//             chain_name, start_time, end_time, start_day, ipset_name);
//         if (system(command) != 0) {
//             snprintf(command, sizeof(command),
//                 "iptables -I %s -m time --timestart %s --timestop %s --weekdays %s -m set --match-set %s src -j DROP",
//                 chain_name, start_time, end_time, start_day, ipset_name);
//             system(command);
//         }
//     } 
//     else {
//         int start_day_index = -1, end_day_index = -1;
//         for (int i = 0; i < 7; i++) {
//             if (strcmp(start_day, days[i]) == 0) {
//                 start_day_index = i;
//             }
//             if (strcmp(end_day, days[i]) == 0) {
//                 end_day_index = i;
//             }
//         }
//         snprintf(command, sizeof(command),
//             "iptables -C %s -m time --timestart %s --timestop 23:59 --weekdays %s -m set --match-set %s src -j DROP 2>/dev/null",
//             chain_name, start_time, start_day, ipset_name);
//         if (system(command) != 0) {
//             snprintf(command, sizeof(command),
//                 "iptables -I %s -m time --timestart %s --timestop 23:59 --weekdays %s -m set --match-set %s src -j DROP",
//                 chain_name, start_time, start_day, ipset_name);
//             system(command);
//         }
//         if (start_day_index != -1 && end_day_index != -1) {
//             char days_in_between[MAX_LENGTH] = "";
//             int next_day = (start_day_index + 1) % 7;
//             while (next_day != end_day_index) {
//                 if (strlen(days_in_between) > 0) {
//                     strcat(days_in_between, ",");
//                 }
//                 strcat(days_in_between, days[next_day]);
//                 next_day = (next_day + 1) % 7;
//             }
//             int size_of_between_day = (int)strlen(days_in_between);
//             if (size_of_between_day != 0) {
//                 snprintf(command, sizeof(command),
//                     "iptables -C %s -m time --timestart 00:00 --timestop 23:59 --weekdays %s -m set --match-set %s src -j DROP 2>/dev/null",
//                     chain_name, days_in_between, ipset_name);
//                 if (system(command) != 0) {
//                     snprintf(command, sizeof(command),
//                         "iptables -I %s -m time --timestart 00:00 --timestop 23:59 --weekdays %s -m set --match-set %s src -j DROP",
//                         chain_name, days_in_between, ipset_name);
//                     system(command);
//                 }
//             }
//         }
//         snprintf(command, sizeof(command),
//             "iptables -C %s -m time --timestart 00:00 --timestop %s --weekdays %s -m set --match-set %s src -j DROP 2>/dev/null",
//             chain_name, end_time, end_day, ipset_name);
//         if (system(command) != 0) {
//             snprintf(command, sizeof(command),
//                 "iptables -I %s -m time --timestart 00:00 --timestop %s --weekdays %s -m set --match-set %s src -j DROP",
//                 chain_name, end_time, end_day, ipset_name);
//             system(command);
//         }
//     }
// }

// void get_list_block_web_info() {
//     int num_line = 0;
//     web_block_info *list = read_web_block_info("./data/ip.txt", &num_line);
//     create_iptables_chain();
//     for(int i = 0; i < num_line; i++) {
//         change_time_to_UTC(list[i].start_time, list[i].start_day);
//         change_time_to_UTC(list[i].end_time, list[i].end_day);
//         char ipset_name[256];
//         snprintf(ipset_name, sizeof(ipset_name), "blocked_%s", list[i].url);
//         create_ipset(ipset_name);
//         add_ip_to_ipset(ipset_name, list[i].ip);
//         if (!check_iptables_rule_exists(ipset_name)) {
//             add_iptables_rule(CHAIN_NAME,ipset_name,list[i].start_day,list[i].start_time,list[i].end_day,list[i].end_time);
//         }
//     }
// }



// void delete__iptable_rules_chain_and_ipset(){
//     snprintf(command, sizeof(command), "iptables -D INPUT -j %s 2>/dev/null", CHAIN_NAME);
//     system(command);
//     system("iptables -F BLOCK_IP_CHAIN");
//     system("iptables -X BLOCK_IP_CHAIN");
//     FILE *file = fopen("./data/ipset.txt", "r");
//     if (file) {
//         char ipset_name[256];
//         while (fgets(ipset_name, sizeof(ipset_name), file)) {
//             ipset_name[strcspn(ipset_name, "\n")] = 0;
//             //snprintf(command, sizeof(command), "ipset destroy %s", ipset_name);
//             snprintf(command, sizeof(command), "ipset destroy %s", ipset_name);
//             system(command);
//         }
//         fclose(file);

//         file = fopen("./data/ipset.txt", "w");
//         if (file) {
//             fclose(file);
//         }
//     }
// }
