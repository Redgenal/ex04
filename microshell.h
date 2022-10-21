#ifndef MICROSHELL_H
#define MICROSHELL_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_list {
    char **strs;
    struct s_list *next;
    struct s_list *prev;
    int     type;
    int     pipes[2];
}   t_list;

typedef struct s_main {
    t_list *exe;
    int    in;
    int    out;
}   t_main;

#endif