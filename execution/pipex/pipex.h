/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 00:38:11 by cboujrar          #+#    #+#             */
/*   Updated: 2024/09/02 15:52:41 by cboujrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_H
#define PIPEX_H

#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<unistd.h>
#include <fcntl.h>
#include<string.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct s_list{
    char *cmd;
    char *arg;
    struct s_list *next;
}t_list;

void first_child(char *av, int fd, int end[2], char *path);
void second_child(char *av,int fd ,  int end[2], char *path);
char	**ft_split(const char *s, char c);
char *get_env();
char	*find_path(char *cmd, char *path);
void execute(char *PATH, char **av, int end[2]);
char	*ft_strjoin(char const *s1, char const *s2);
char	*find_path(char *cmd, char *path);
t_list *parse_input(char *s);
void check_cmd(char *current, char *next, t_list **list);
void	append(t_list **list, char* cmd, char *arg);
int	ft_lstsize(t_list *lst);
char **create_arg(t_list *list,char *path);
void multipipe(t_list *list);
void execute_multipipe(char *path, int end[2], t_list *list);
void execute_cmd(t_list *list);
void child_process(t_list *list, int end[2], int prev_end[2], char * path);
void set_prev_end(int prev_end[2], int end[2]);


#endif