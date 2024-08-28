/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 00:38:11 by cboujrar          #+#    #+#             */
/*   Updated: 2024/08/28 02:31:28 by cboujrar         ###   ########.fr       */
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

void first_child(char *av, int fd, int end[2], char *path);
void second_child(char *av,int fd ,  int end[2], char *path);
char	**ft_split(const char *s, char c);
char *get_env();
void execute(char *PATH, char **av, int end[2]);
char	*ft_strjoin(char const *s1, char const *s2);
char	*find_path(char *cmd, char *path);


#endif