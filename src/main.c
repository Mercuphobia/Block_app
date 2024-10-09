#include <stdio.h>
#include <stdlib.h>
#include "parsers_option.h"
#include "file_process.h"
#include "get_data.h"
#include "parsers_data.h"
#include "block_web.h"


#define SRC_DIR "../test_app/data/data.txt"
#define DES_DIR "../block_app/data/data.txt"
#define BLOCK_WEB "./data/block_web.txt"
#define IP_FILE "./data/ip.txt"

int main(int argc, char *argv[]) {
    parsers_option(argc, argv);
    transfer_data(SRC_DIR, DES_DIR);
    //printf_block_web();
    //print_data();
    printf_to_file(IP_FILE);
    create_ipset();
    block_ips_from_file(IP_FILE);
}
