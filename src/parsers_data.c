#include <stdio.h>
#include <string.h>
#include "parsers_data.h"
#include "log.h"

char line[256];

website_block* read_block_web(const char *filename, int *line_count) {
    website_block *list_block_web = NULL; // Con trỏ cho mảng động
    *line_count = 0;        // Khởi tạo số dòng đã đọc
    int capacity = 10;      // Số lượng dòng tối đa ban đầu

    // Cấp phát bộ nhớ cho mảng động
    list_block_web = malloc(capacity * sizeof(website_block));
    if (list_block_web == NULL) {
        perror("Không thể cấp phát bộ nhớ");
        return NULL;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Không thể mở file");
        free(list_block_web);
        return NULL;
    }

    // char line[256]; // Biến để lưu từng dòng đọc được

    while (fgets(line, sizeof(line), file)) {
        // Tăng kích thước mảng nếu cần
        if (*line_count >= capacity) {
            capacity *= 2; // Gấp đôi kích thước mảng
            list_block_web = realloc(list_block_web, capacity * sizeof(website_block));
            if (list_block_web == NULL) {
                perror("Không thể cấp phát lại bộ nhớ");
                fclose(file);
                return NULL;
            }
        }
        line[strcspn(line, "\n")] = '\0';
        char *token = strtok(line, ", ");
        if (token != NULL) {
            strncpy(list_block_web[*line_count].url, token, MAX_LENGTH);
        }
        token = strtok(NULL, " ");
        if (token != NULL) {
            strncpy(list_block_web[*line_count].start_day, token, MAX_LENGTH);

        } else {
            list_block_web[*line_count].start_day[0] = '\0';
        }
        token = strtok(NULL, ", ");
        if (token != NULL) {
            strncpy(list_block_web[*line_count].start_time, token, MAX_LENGTH);
        } else {
            list_block_web[*line_count].start_time[0] = '\0';
        }
        token = strtok(NULL, " ");
        if (token != NULL) {
            strncpy(list_block_web[*line_count].end_day, token, MAX_LENGTH);
        } else {
            list_block_web[*line_count].end_day[0] = '\0';
        }

        token = strtok(NULL, " ");
        if (token != NULL) {
            strncpy(list_block_web[*line_count].end_time, token, MAX_LENGTH);
        } else {
            list_block_web[*line_count].end_time[0] = '\0';
        }
        (*line_count)++;
    }
    fclose(file);
    return list_block_web;
}

void printf_block_web(){
    int line_count = 0;
    website_block *list = read_block_web("./data/block_web.txt",&line_count);
    for(int i=0;i<line_count;i++){
        printf("URL: %s\n", list[i].url);
        printf("Start Day: %s\n", list[i].start_day);
        printf("Start Time: %s\n", list[i].start_time);
        printf("End Day: %s\n", list[i].end_day);
        printf("End Time: %s\n", list[i].end_time);
        printf("\n");
    }   
}

website_info* read_data_file(const char *filename, int *entry_count) {
    website_info *list_web = NULL;
    *entry_count = 0;
    int capacity = 10;
    list_web = malloc(capacity * sizeof(website_info));
    if (list_web == NULL) {
        perror("Không thể cấp phát bộ nhớ");
        return NULL;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Không thể mở file");
        free(list_web);
        return NULL;
    }
    char line[MAX_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (*entry_count >= capacity) {
            capacity *= 2;
            list_web = realloc(list_web, capacity * sizeof(website_info));
            if (list_web == NULL) {
                perror("Không thể cấp phát lại bộ nhớ");
                fclose(file);
                return NULL;
            }
        }
        sscanf(line, "TIME: %s DATE: %s", list_web[*entry_count].time, list_web[*entry_count].date);
        fgets(line, sizeof(line), file);
        sscanf(line, "Name: %s", list_web[*entry_count].url);
        fgets(line, sizeof(line), file);
        sscanf(line, "IPv4 Address: %s", list_web[*entry_count].ip); 
        (*entry_count)++;
        fgets(line, sizeof(line), file);
    }
    fclose(file);
    return list_web;
}

void print_data() {
    int entry_count = 0;
    website_info *list_web = read_data_file("./data/data.txt", &entry_count);
    
    if (list_web == NULL) {
        return;
    }
    for (int i = 0; i < entry_count; i++) {
        printf("TIME: %s\n", list_web[i].time);
        printf("DATE: %s\n", list_web[i].date);
        printf("URL: %s\n", list_web[i].url);
        printf("IPv4: %s\n", list_web[i].ip);
        printf("\n");
    }
    
    free(list_web);
}

web_block_info* get_ip(int *out_count) {
    int line_count = 0;
    website_block *list_block = read_block_web("./data/block_web.txt", &line_count);
    int entry_count = 0;
    website_info *list_info = read_data_file("./data/data.txt", &entry_count);

    if (list_block == NULL || list_info == NULL) {
        *out_count = 0;
        return NULL;
    }
    char printed_ips[entry_count][MAX_LENGTH];
    int printed_count = 0;
    int result_capacity = 10;
    int result_count = 0;
    web_block_info *result_list = malloc(result_capacity * sizeof(web_block_info));
    if (result_list == NULL) {
        perror("Không thể cấp phát bộ nhớ cho result_list");
        free(list_block);
        free(list_info);
        *out_count = 0;
        return NULL;
    }

    for (int i = 0; i < line_count; i++) {
        for (int j = 0; j < entry_count; j++) {
            // So sánh trường `url` của `website_block` và `website_info`
            if (strcmp(list_block[i].url, list_info[j].url) == 0) {
                // Kiểm tra xem IP đã được in ra chưa
                int already_printed = 0;
                for (int k = 0; k < printed_count; k++) {
                    if (strcmp(printed_ips[k], list_info[j].ip) == 0) {
                        already_printed = 1;
                        break;
                    }
                }
                // Nếu chưa in, thêm IP vào danh sách và lưu vào `result_list`
                if (!already_printed) {
                    strncpy(printed_ips[printed_count], list_info[j].ip, MAX_LENGTH);
                    printed_count++;

                    // Tăng kích thước mảng nếu cần
                    if (result_count >= result_capacity) {
                        result_capacity *= 2;
                        result_list = realloc(result_list, result_capacity * sizeof(web_block_info));
                        if (result_list == NULL) {
                            perror("Không thể cấp phát lại bộ nhớ cho result_list");
                            free(list_block);
                            free(list_info);
                            *out_count = 0;
                            return NULL;
                        }
                    }
                    // Thêm thông tin vào `result_list`
                    strncpy(result_list[result_count].url, list_block[i].url, MAX_LENGTH);
                    strncpy(result_list[result_count].ip, list_info[j].ip, MAX_LENGTH);
                    strncpy(result_list[result_count].start_day, list_block[i].start_day, MAX_LENGTH);
                    strncpy(result_list[result_count].start_time, list_block[i].start_time, MAX_LENGTH);
                    strncpy(result_list[result_count].end_day, list_block[i].end_day, MAX_LENGTH);
                    strncpy(result_list[result_count].end_time, list_block[i].end_time, MAX_LENGTH);
                    result_count++;
                }
            }
        }
    }
    // Giải phóng bộ nhớ đã cấp phát cho các danh sách gốc
    free(list_block);
    free(list_info);
    // Trả về số lượng kết quả tìm được
    *out_count = result_count;
    return result_list;
}

void printf_to_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Không thể mở file để ghi");
        return;
    }
    int result_count = 0;
    web_block_info *results = get_ip(&result_count);
    if (results == NULL) {
        fprintf(stderr, "No data write to file\n");
        fclose(file);
        return;
    }
    for (int i = 0; i < result_count; i++) {
        fprintf(file, "%s\n", results[i].ip);
    }
    fclose(file);
    free(results);
}


