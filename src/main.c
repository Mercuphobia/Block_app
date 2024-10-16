#include <stdio.h>
#include <stdlib.h>
#include "parsers_option.h"
#include "file_process.h"
#include "get_data.h"
#include "parsers_data.h"
#include "log.h"
#include "block_ip.h"


#define SRC_DIR "../test_app/data/data.txt"
#define DES_DIR "../block_app/data/data.txt"
#define BLOCK_WEB "./data/block_web.txt"
#define IP_FILE "./data/ip.txt"
#define CHECK_FILE "./data/check.txt"

int main(int argc, char *argv[]) {
    if(argc > 1){
        parsers_option(argc, argv);
    }
    else {
        transfer_data(SRC_DIR, DES_DIR);
        printf_to_file(IP_FILE);
        //printf_ip_and_time_to_console();
        //create_ipset();
        //block_ips_from_file(IP_FILE);
        //block_ips_by_time(IP_FILE);
        //create_iptables_chain();
        //get_list_block_web_info();
        clear_check_file(IP_FILE);
        clear_file_to_run(CHECK_FILE);
        get_list();
        run();
    }
}
