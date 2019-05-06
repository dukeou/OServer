#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "btree.h"
#include "list.h"
#include "stack.h"
#include "logger.h"
#include "http_header.h"
#include "http_version.h"
#include "http_url.h"
#include "http_method.h"
#include "sync_queue.h"
#include "thread_pool.h"
#include "http_request.h"
#include "http_response.h"
#include "object_pool.h"
#include "server_config.h"
#include "server.h"

int main(int argc, char * const argv[])
{
    server_t server;
    //logger_init("/dev/tty", l_og_l_evel_d_ebug);
    http_header_hash_all_fields();
//    int x = http_header_binary_search("Origin");
//    if(x != -1)
//    {
//        l_og(e_log_level_info, "Find %s for Origin\n", http_header_fields[x].str);
//    }
    if(0 != server_config_read(&server.config, argc, argv))
        return 1;
    server_entry(&server);
    return 0;
}
