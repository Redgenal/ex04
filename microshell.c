#include "microshell.h"
#include <stdio.h>

int ft_strlen(char *str) {
    int i = 0;

    while (str[i] != '\0')
        i++;
    return (i);
}

void exit_fatal(void)
{
	write(2, "error: fatal\n", ft_strlen("error: fatal\n"));
	exit(EXIT_FAILURE);
}

void exit_execve(char *str)
{
	write(2, "error: cannot execute ", ft_strlen("error: cannot execute "));
	write(2, str, ft_strlen(str));
	write(2, "\n", 1);
	exit(EXIT_FAILURE);
}

int exit_cd_1(void)
{
	write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
	return (EXIT_FAILURE);
}

int exit_cd_2(char *str)
{
	write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
	write(2, str, ft_strlen(str));
	write(2, "\n", 1);
	return (EXIT_FAILURE);
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

    if (list->type == 1 || (list->prev && list->prev->type == 1))
        if (pipe(list->pipes))
            exit_fatal();
    pid = fork();
    if (pid < 0)
        exit_fatal();
    else if (pid == 0) {
        if (list->type == 1 && (dup2(list->pipes[1], 1)) < 0)
            exit_fatal();
        if (list->prev && list->prev->type == 1 && (dup2(list->prev->pipes[0], 0)) < 0)
            exit_fatal();
        execve(list->strs[0], list->strs, envp);
        exit_execve(list->strs[0]);
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

void    ft_clean_all(t_main *main_struct) {
    int i = -1;
    t_list  *tmp;

    while(main_struct->first) {
        i = -1;
        tmp = main_struct->first->next;
        while(main_struct->first->strs[++i])
            free(main_struct->first->strs[i]);
        free(main_struct->first->strs);
        free(main_struct->first);
        main_struct->first = tmp;
    }
    free(main_struct);
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
        while(argv[i] && (strcmp(argv[i], "|") != 0) && (strcmp(argv[i], ";") != 0)) {
            i++;
        }
        ft_push_back(&main_struct->exe, ft_create_list(argv, start, i - start));
        i++;
    }
    main_struct->first = main_struct->exe;
    while (main_struct->exe != NULL) {
        if (strcmp(main_struct->exe->strs[0], "cd") == 0) {
            if (main_struct->exe->strs[1] == NULL || main_struct->exe->strs[2])
                exit_cd_1();
            if (chdir(main_struct->exe->strs[1]) != 0)
                exit_cd_2(main_struct->exe->strs[1]);
        }
        else {
            ft_exequtor(main_struct->exe, envp);
        }
        main_struct->exe = main_struct->exe->next;
    }
    ft_clean_all(main_struct);
    return (0);
}