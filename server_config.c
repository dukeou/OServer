#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "logger.h"
#include "server_config.h"

#define VERSION "0.1"
#define HELP_TXT \
" *************************************************************************\n" \
" * %s -p <port> -r <root path>\n" \
" * \n" \
" * -p,--port <port>                 : port.\n" \
" * -r,--root <root path>            : root path.\n" \
" * -h,--help                        : display this help text.\n" \
" * \n" \
" * Bug report\n" \
" *     ou.bai@hp.com\n" \
" * \n" \
" * Version\n" \
" *     %s\n" \
" *************************************************************************\n"
static struct option optLongTable[] = 
{
    {
        "port",
        1,
        NULL,
        'p'
    },
    {
        "root",
        1,
        NULL,
        'r'
    },
    {
        "help",
        0,
        NULL,
        'h'
    },
};
int server_config_read(server_config_t *cfg, int argc, char * const argv[])
{
    int opt;
    char *tmp;
    struct stat file_stat;

    tmp = NULL;
    opt = -1;
    cfg->root[0] = '\0';
    while((opt = getopt_long(argc, argv, "p:r:h", optLongTable, NULL)) != -1)
    {
        switch(opt)
        {
            case 'p':
                cfg->port = strtol(optarg, &tmp, 10);
                break;
            case 'r':
                if(stat(optarg, &file_stat) == 0)
                    realpath(optarg, cfg->root);
                else
                    l_og_error(optarg);
                break;
            case 'h':
                server_config_help(argv[0]);
                return 1;
            default:
                server_config_help(argv[0]);
                return 1;
        }
    }
    if(tmp == NULL || *tmp != '\0')
        cfg->port = O_SERVER_DEFAULT_PORT;
    if(cfg->root[0] == '\0')
        getcwd(cfg->root, sizeof(cfg->root));
    cfg->num_regular_threads = 10;
    cfg->num_max_threads = 20;
    cfg->num_max_connections = 1024;
    cfg->stale_connection_timeout_sec = 120;
    cfg->backlog = 50;
    return 0;
}
void server_config_help(char *program)
{
    /* Get program name */
    char *prgnm = strrchr(program, '/');
    if(!prgnm)
        prgnm = program;
    else
        ++prgnm;
    fprintf(stdout, HELP_TXT, prgnm, VERSION);
    fflush(stdout);
}
