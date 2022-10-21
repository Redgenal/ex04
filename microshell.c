#include "microshell.h"
#include <stdio.h>

int ft_strlen(char *str) {
    int i = 0;

    while (str[i] != '\0')
        i++;
    return (i);
}

char    *ft_strdup(char *str) {
    int i = -1;
    char *new;

    new = malloc (sizeof(char) * (ft_strlen(str) + 1));
    while (++i < ft_strlen(str))
        new[i] = str[i];
    new[i] = '\0';
    return (new);
}

t_list  *ft_create_list(char **argv, int start, int count){
    t_list  *new;
    int i = 0;
    
    new = (t_list *) malloc(sizeof(*new));
    new->next = NULL;
    new->prev = NULL;
    new->type = -1;
    new->strs = malloc(sizeof(char *) * (count + 1));
    while (i < count) {
        new->strs[i] = ft_strdup(argv[start + i]);
        i++;
    }
    new->strs[i] = NULL;
    if (argv[start + i] == NULL)
        new->type = 0;
    else if (argv[start + i][0] == '|')
        new->type = 1;
    else if(argv[start + i][0] == ';')
        new->type = 2;
    return (new);
}

void    ft_push_back(t_list **list, t_list *new) {
    t_list *p;

    if (list) {
        if (*list) {
            p = *list;
            while (p->next)
                p = p->next;
            p->next = new;
            new->prev = p;
        }
        else {
            *list = new;
        }
    }
}

void    ft_exequtor(t_list *list, char **envp) {
    pid_t   pid;

    pid = fork();
    if (pid < 0)
        exit(0);
    else if (pid == 0) {
        if (list->type == 1 && (dup2(list->pipes[1], 1)) < 0)
            return;
        if (list->prev && list->prev->type == 1 && (dup2(list->prev->pipes[0], 0)) < 0)
            return;
        execve(list->strs[0], list->strs, envp);
    }
    else {
        waitpid(pid, NULL, 0);
        if (list->type == 1 || list->prev->type == 1) {
            close(list->pipes[1]);
            if (list->type != 1)
                close(list->pipes[0]);
        }
        if (list->prev && list->prev->type == 1)
			close(list->prev->pipes[0]);
    }
}

int main(int argc, char **argv, char **envp) {
    t_main  *main_struct;
    int i = 1;
    int start = 0;

    if (argc == 1)
        return (0);
    (void) envp;
    main_struct = (t_main *)malloc(sizeof(*main_struct));
    main_struct->in = dup(STDIN_FILENO);
    main_struct->out = dup(STDOUT_FILENO);
    main_struct->exe = NULL;
    while (i < argc) {
        start = i;
        while(argv[i] && argv[i][0] != '|' && argv[i][0] != ';') {
            i++;
        }
        ft_push_back(&main_struct->exe, ft_create_list(argv, start, i - start));
        i++;
    }
    while (main_struct->exe != NULL) {
        if (strcmp(main_struct->exe->strs[0], "cd") == 0) {
            if (main_struct->exe->strs[1] == NULL || main_struct->exe->strs[2]) {
                printf("bad cd1\n");
                return(1);
            }
            if (chdir(main_struct->exe->strs[1]) != 0) {
                printf("bad cd2");
                return (2);
            }
        }
        else {
            ft_exequtor(main_struct->exe, envp);
        }
        main_struct->exe = main_struct->exe->next;
    }
    return (0);
}