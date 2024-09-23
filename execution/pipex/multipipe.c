/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multipipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 09:44:23 by cboujrar          #+#    #+#             */
/*   Updated: 2024/09/02 15:55:28 by cboujrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

// int main(int ac, char **av)
// {
//     (void) ac;
//     (void)av;
//     t_list *list;
    
//     while(1)
//     {  
//         list = parse_input(readline(">>"));
//         if(ft_lstsize(list) == 1)
//             execute_cmd(list);
//         else
//             multipipe(list);
//     }
// }

t_list *parse_input(char *s)
{
    char **av;
    t_list *list;
    int i;
    
    av = ft_split(s, ' ');
    i = 0;
    list = NULL;
    while(av[i] != NULL)
    {
        if(av[i][0] == '|')
            i++;
        else if(av[i + 1] != NULL && av[i + 1][0] != '|' )
        {
            check_cmd(av[i], av[i + 1] ,&list);
            i+=2;
        }
        else 
        {
            check_cmd(av[i], NULL, &list);
            i++;   
        }
    }
    return(list);
}

void check_cmd(char *current, char *next, t_list **list)
{
    if(next == NULL)
        append(list, current, NULL);
    else if(current[0] != '-' && current[0] != '|' )
        append(list, current, next);
        
}


void	append(t_list **list, char* cmd, char *arg)
{
	t_list	*new_node;
	t_list	*current;

	current = NULL;
	new_node = malloc(sizeof(t_list));
	if (!new_node)
		return ;
	new_node->cmd = cmd;
    new_node->arg = arg;
	new_node->next = NULL;
	if (*list == NULL)
		*list = new_node;
	else
	{
		current = *list;
		while (current->next != NULL)
		{
			current = current->next;
		}
		current->next = new_node;
	}
}

int	ft_lstsize(t_list *lst)
{
	int	size;

	size = 0;
	while (lst)
	{
		size++;
		lst = lst->next;
	}
	return (size);
}

char **create_arg(t_list *list,char *path)
{
    char **arg;
    
    arg = malloc(sizeof(char *) * 3);
    arg[0] = path;
    arg[1] = list->arg;
    arg[2] = NULL;
    return(arg); 
}

void multipipe(t_list *list)
{
    int end[2];
    char	*PATH;

	PATH = get_env();
    execute_multipipe(PATH, end, list);
}

void execute_multipipe(char *path, int end[2], t_list *list)
{
    pid_t pid;
    int status;
    int prev_end[2];
    
    prev_end[0] = -1;
    prev_end[1] = -1;
    while(list)
    {
        if(list->next)
        {
            if(pipe(end) == -1)
                exit(EXIT_FAILURE);
        }
        pid = fork();
        if(pid == -1)
            exit(EXIT_FAILURE);
        else if(pid == 0)
            child_process(list, end, prev_end, path);
        set_prev_end(prev_end, end);
        list = list->next;
    }
    while(waitpid(-1, &status, 0) > 0);
    close(end[0]);
    close(end[1]);
}

void child_process(t_list *list, int end[2], int prev_end[2], char * path)
{
    char **arg;

        if(prev_end[0] != -1)
            {
                close(prev_end[1]);
                dup2(prev_end[0], STDIN_FILENO);
                close(prev_end[0]);
            }
            if (list->next)
            {
                close(end[0]);
                dup2(end[1], STDOUT_FILENO);
                close(end[1]);
            }
            path = find_path(list->cmd, path);
            arg = create_arg(list, path);
            execve(path, arg, NULL);
            exit(EXIT_FAILURE);
}

void set_prev_end(int prev_end[2], int end[2])
{
        if (prev_end[0] != -1)
        {
            close(prev_end[0]);
            close(prev_end[1]);
        }
        prev_end[0] = end[0];
        prev_end[1] = end[1];
}

void execute_cmd(t_list *list)
{
    char *path;
    char **arg;
    pid_t pid;
    int status;
    
    pid = fork();
    if(pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        path = get_env("PATH");
        path = find_path(list->cmd, path);
        arg = create_arg(list, path);
        if(execve(path, arg, NULL) == -1)
            printf("command not found: %s\n", list->cmd);
    }
    waitpid(pid, &status, 0);
}

