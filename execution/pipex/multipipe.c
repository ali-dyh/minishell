/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multipipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 09:44:23 by cboujrar          #+#    #+#             */
/*   Updated: 2024/08/28 13:17:00 by cboujrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

int main(int ac, char **av)
{
    char *s;
    (void) ac;
    (void)av;
    t_list *list;
    
    while(1)
    {  
        printf("minishell->");
        s = readline(NULL);
        // printf("--> %s\n", s);
        list = parse_input(s);
        if(ft_lstsize(list) == 2)
            pipex(list);
        else if (ft_lstsize(list) > 2)
        {
            multipipe(list);
        }
    }
}

t_list *parse_input(char *s)
{
    char **av;
    t_list *list;
    int i;
    
    av = ft_split(s, ' ');
    // i = 0;
    // while(av[i])
    // {
    //     printf("--->%s\n", av[i]);
    //     i++;
    // }
    // printf("--->%s\n", av[i]);
    i = 0;
    list = NULL;
    while(av[i] != NULL)
    {
        // printf("--->%s\n", av[i]);
        // printf("--->%c\n", av[i + 1][0]);
        if(av[i][0] == '|')
            i++;
        else if(av[i + 1] != NULL && av[i + 1][0] != '|' && av[i + 1][0] == '-')
        {
            // printf("I am here\n");
            check_cmd(av[i], av[i + 1] ,&list);
            i+=2;
        }
        else 
        {
            check_cmd(av[i], NULL, &list);
            i++;   
        }
        // printf("%d\n", i);
    }
        // while(list)
        // {
        //     printf("-->%s\n", list->cmd);
        //     printf("-->%s\n", list->arg);
        //     list = list->next;
        // }
    return(list);
}

void check_cmd(char *current, char *next, t_list **list)
{
    if(next == NULL)
        append(list, current, NULL);
    else if(current[0] != '-' && current[0] != '|' && next[0] == '-')
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

void pipex(t_list *list)
{
    int		end[2];
	char	*PATH;

	PATH = get_env();

	execute_1(PATH, list, end);
}

void	execute_1(char *PATH, t_list *list, int end[2])
{
	pid_t	pid1;
	pid_t	pid2;
    t_list  *tmp;
    int status;

    
	if (pipe(end) == -1)
		exit(EXIT_FAILURE);
	pid1 = fork();
	if (pid1 == -1)
		exit(EXIT_FAILURE);
	else if (pid1 == 0)
		first_child_1(list , end, PATH);
	pid2 = fork();
    tmp = list;
	if (pid2 == -1)
		exit(EXIT_FAILURE);
	else if (pid2 == 0)
    { 
        tmp = tmp->next;
		second_child_1(tmp, end, PATH);
    }
    close(end[0]);
    close(end[1]);
	waitpid(pid1, &status, 0);
	waitpid(pid2, &status, 0);
}

void	first_child_1(t_list *list, int end[2], char *path)
{
	char	**arg;

	arg = create_arg(list);
	if (dup2(end[1], STDOUT_FILENO) == -1)
		exit(EXIT_FAILURE);
    close(end[0]);
    close(end[1]);
	path = find_path(list->cmd, path);
	if (execve(path, arg, NULL) == -1)
		exit(EXIT_FAILURE);
}

void	second_child_1(t_list *list, int end[2], char *path)
{
	char	**arg;

	arg = create_arg(list);
	if (dup2(end[0], STDIN_FILENO) == -1)
		exit(EXIT_FAILURE);
	close(end[1]);
    close(end[0]);
	path = find_path(list->cmd, path);
	if (execve(path, arg, NULL) == -1)
		exit(EXIT_FAILURE);
}

char **create_arg(t_list *list)
{
    char **arg;

    arg = malloc(sizeof(char *) * 3);
    arg[0] = list->cmd;
    arg[1] = list->arg;
    arg[2] = NULL;
    // int i = 0;
    // while(arg[i])
    // {
    //     printf("---> %s\n", arg[i]);
    //     i++;
    // }
    return(arg); 
}

void multipipe(t_list *list)
{
    int end[2];
    char	*PATH;

	PATH = get_env();
    execute_2(PATH, end, list);
}

void execute_2(char *path, int end[2], t_list *list)
{
    pid_t pid1;
    pid_t pid2;
    // t_list *tmp;
    int status;

    if(pipe(end) == -1)
        exit(EXIT_FAILURE);
    // execute_1(path, list, end);
    // list = list->next;
    // list = list->next;
    while(list && list->next)
    {
        pid1 = fork();
        if(pid1 == -1)
            exit(EXIT_FAILURE);
        else if(pid1 == 0)
            execute_child(list, end, path);
        pid2 = fork();
    
        if(pid2 == -1)
            exit(EXIT_FAILURE);
        else if (pid2 == 0)
        {
            list = list->next;
            execute_child(list, end, path);
        }  
        close(end[0]);
        close(end[1]);
        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);
    }
}

void execute_child(t_list *list, int end[2], char *path)
{
    char	**arg;

	arg = create_arg(list);
	if (dup2(end[1], STDOUT_FILENO) == -1)
		exit(EXIT_FAILURE);
    if (dup2(end[0], STDIN_FILENO) == -1)
		exit(EXIT_FAILURE);
    close(end[0]);
    close(end[1]);
	path = find_path(list->cmd, path);
	if (execve(path, arg, NULL) == -1)
		exit(EXIT_FAILURE);
}
