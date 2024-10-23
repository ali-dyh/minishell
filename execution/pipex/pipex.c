/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 02:32:05 by cboujrar          #+#    #+#             */
/*   Updated: 2024/08/31 23:21:17 by cboujrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

// int	main(int ac, char **av)
// {
// 	int		end[2];
// 	char	*PATH;

// 	PATH = get_env();
// 	if (ac == 5)
// 		execute(PATH, av, end);
// 	else
// 		printf("error");
// }

// void	execute(char *PATH, char **av, int end[2])
// {
// 	int		fd1;
// 	int		fd2;
// 	pid_t	pid1;
// 	pid_t	pid2;

// 	fd1 = open(av[1], O_RDONLY);
// 	fd2 = open(av[4], O_WRONLY);
// 	if (pipe(end) == -1)
// 		exit(EXIT_FAILURE);
// 	pid1 = fork();
// 	if (pid1 == -1)
// 		exit(EXIT_FAILURE);
// 	else if (pid1 == 0)
// 		first_child(av[2], fd1, end, PATH);
// 	pid2 = fork();
// 	if (pid2 == -1)
// 		exit(EXIT_FAILURE);
// 	else if (pid2 == 0)
// 		second_child(av[3], fd2, end, PATH);
// }

// char	*get_env(void)
// {
// 	char	*PATH;
// 	char	**t;

// 	PATH = getenv("PATH");
// 	t = ft_split(PATH, ':');
// 	PATH = ft_strjoin(t[6], "/");
// 	return (PATH);
// }

// void	first_child(char *av, int fd, int end[2], char *path)
// {
// 	char	**arg;
// 	pid_t	pid1;
// 	int		status;

// 	pid1 = getpid();
// 	arg = ft_split(av, ' ');
// 	if (dup2(fd, STDIN_FILENO) == -1)
// 		exit(EXIT_FAILURE);
// 	if (dup2(end[1], STDOUT_FILENO) == -1)
// 		exit(EXIT_FAILURE);
// 	close(end[0]);
// 	path = find_path(av, path);
// 	if (execve(path, arg, NULL) == -1)
// 		exit(EXIT_FAILURE);
// 	waitpid(pid1, &status, 0);
// 	close(fd);
// 	close(end[0]);
// }

// void	second_child(char *av, int fd, int end[2], char *path)
// {
// 	char	**arg;
// 	pid_t	pid2;
// 	int		status;

// 	pid2 = getpid();
// 	arg = ft_split(av, ' ');
// 	if (dup2(fd, STDOUT_FILENO) == -1)
// 		exit(EXIT_FAILURE);
// 	if (dup2(end[0], STDIN_FILENO) == -1)
// 		exit(EXIT_FAILURE);
// 	close(end[1]);
// 	path = find_path(av, path);
// 	if (execve(path, arg, NULL) == -1)
// 		exit(EXIT_FAILURE);
// 	waitpid(pid2, &status, 0);
// 	close(fd);
// 	close(end[1]);
// }

size_t	ft_strlcpy(char *dst, char *src, int size)
{
	int	i;

	i = 0;
	if (size != 0)
	{
		while (i < (size - 1) && src[i] != '\0')
		{
			dst[i] = src[i];
			i++;
		}
		dst[i] = '\0';
	}
	return (strlen(src));
}

char	*ft_substr(char *s, int start, int len)
{
	int	finish;
	char	*newstr;
	int slen;

	if (!s)
		return (0);
	slen = strlen(s);
	finish = 0;
	if (start >= slen)
		return (strdup(""));
	finish = slen - start;
	if (finish > len)
		finish = len;
	newstr = (char *)malloc(finish + 1);
	if (!newstr)
		return (0);
	ft_strlcpy(newstr, &s[start], finish + 1);
	return (newstr);
}

// char	*find_path(char *cmd, char *path)
// {
// 	int		i;

// 	i = 0;
// 	while (cmd[i] != ' ' && cmd[i])
// 		i++;
// 	if (cmd[i] != '\0')
// 		cmd = ft_substr(cmd, 0, i);
// 	path = ft_strjoin(path, cmd);
// 	if (access(path, X_OK) == 0)
// 		return (path);
// 	return (NULL);
// }