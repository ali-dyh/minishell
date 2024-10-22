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


typedef struct s_echo{
    int option ;
    char *str;
    struct s_echo *next;
} t_echo;

t_echo *check_echo(char *s);

int main()
{
    char *s;
    // t_echo *list;
    
    while(1)
    {  
        s = readline(">>");
        if(ft_strcmp(s, "cd") == 0)
            bultin_cd(NULL);
        else if(ft_strcmp(s, "pwd") == 0)
            bultin_pwd();
        else if (ft_strcmp(s, "exit") == 0)
            exit(50);
        // list = check_echo(s);
        // bultin_echo(&list->str, list->option);
    }
}

// t_echo *check_echo(char *s)
// {
//     int i = 5;
//     int j;
//     t_echo *args;

//     args = malloc(sizeof(t_echo));
//     if(s[i] == '-')
//         i++;
//     if(s[i] == 'n')
//     {
//         args->option = 1;
//         i++;
//     }
//     else 
//         args->option = 0;
//     j = i;
//     while(s[i])
//         i++;
//     args->str = malloc(sizeof(char) * i);
//     i = 0;
//     while(s[j])
//     {
//         args->str[i] = s[j];
//         i++;
//         j++;
//     }
//     args->str[i] = '\0';
//     args->next = NULL;
//     return(args);

// }

void bultin_cd(char *path)
{
    if(path == NULL || !ft_strcmp(path, "") || !ft_strcmp(path, "~"))
        path = getenv("HOME");

    // printf("--->%s\n", path);
    if(chdir(path) != 0) //the starting point for path searches for pathnames not beginning with '/'
    {
        perror("cd ");
    }
    //TODO: change the current directory(pwd) and the last directory(oldpwd) in env
    //TODO: handlle cd -  chdir(oldpwd)
    // the `cd -` : switch between the current and previous directories. 
}

void bultin_pwd()
{
    char buffer[PATH_MAX];

    if(getcwd(buffer, PATH_MAX) != NULL)
        printf("%s\n", buffer);
    else
        perror("pwd");
    //TODO: exit status 0 on success or -1 on error
}

void bultin_echo(char **args, int option)
{
    int newline;

    newline = 1;
    if(option != 0)
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
    //TODO:check the option 
    //TODO: test echo 
}

//exit argument must be between 0 and 255
//If no parameter is provided, the command returns the exit status of the last executed command.

void builtin_exit(int exit_status)
{
    //TODO:check the exit_status
    if (exit_status < 0 || exit_status >= 255)
        exit_status = 0;
    printf("exit\n");
    exit(exit_status);
}


void builtin_export()
{
    //set the key and value , first we have to search in the list if there is the variable just replace the value
    //parse the key should start with alphabet or _
}
/*
exit status 
builin 1
exit -> case 2
error -> 1
success - > 0
*/


int	ft_strcmp(char *s1,char *s2)
{
    int i;

    i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return (s1[i] - s2[i]);
}
