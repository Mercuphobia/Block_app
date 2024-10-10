#ifndef BLOCK_WEB_H
#define BLOCK_WEB_H

void block_ips_from_file(const char *filename);
void create_ipset();
void printf_ip_and_time_to_console();

void block_ips_by_time(const char *filename);

#endif // BLOCK_WEB_H

