/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builthing.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 15:34:49 by marvin            #+#    #+#             */
/*   Updated: 2024/09/23 15:34:49 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include"pipex/pipex.h"

int main()
{
    char *s;
    
    while(1)
    {  
        s = readline(">>");
        if(ft_strcmp(s, "cd") == 0)
            bultin_cd("..");
        else if(ft_strcmp(s, "pwd") == 0)
            bultin_pwd();
        else if (ft_strcmp(s, "exit") == 0)
    }
}

void bultin_cd(char *path)
{
    if(path == NULL || !ft_strcmp(path, "") || !ft_strcmp(path, "~"))
        path = getenv("HOME");

    // printf("--->%s\n", path);
    if(chdir(path) != 0) //the starting point for path searches for pathnames not beginning with '/'
    {
        perror("cd ");
    }
    //TODO: change the current directory and the last directory in env
    //TODO: handlle cd - 
    // the `cd -` : switch between the current and previous directories. 
}

void bultin_pwd()
{
    char buffer[PATH_MAX + 1];

    if(getcwd(buffer, PATH_MAX + 1) != NULL)
        printf("%s\n", buffer);
    else
        perror("pwd");
    //TODO: exit status 0 on success or -1 on error
}

void bultin_echo(char **args, char *option)
{
    int newline;

    newline = 1;
    if(option != NULL && !ft_strcmp(option, "-n"))
        newline = 0;
    while(*args)
    {
        printf("%s", *args);
        args++;
        if(args != NULL)
            printf(" ");
    }
    if(newline == 1)
        printf("\n");
    //TODO: test echo 
}


int	ft_strcmp(char *s1,char *s2)
{
    int i;

    i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return (s1[i] - s2[i]);
}
