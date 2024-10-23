/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/19 12:16:43 by marvin            #+#    #+#             */
/*   Updated: 2024/10/23 19:05:40 by cboujrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../parsing/parser.h"

void setup_io_redirection(t_cmd_io *cmd_io);
void execute_pipe(t_op_data *op_data, int nb_pipes);
void execute_operator(t_op_data *op_data, int nb_pipes);
void execute_cmd(t_cmd_data *cmd_data);
char **ft_split(const char *s, char c);
size_t ft_strlcpy(char *dst, char *src, int size);
char *ft_substr(char *s, int start, int len);
char *ft_strjoin(char const *s1, char const *s2);
int calcule_pipes(t_node *node);
void child_process(t_node *node, int end[2], int prev_end[2], int nb_pipes);
void set_prev_end(int prev_end[2], int end[2]);

// enum e_syntax_error
// {
//     NO_SYN_ERR,
//     UNEXPECTED_EOF,
//     UNEXPECTED_TOKEN,
//     WTF // malloc failure
// };

// enum e_node_type
// {
//     NT_CMD = 1,
//     NT_OP
// };

// enum e_op_type
// {
//     OT_OR = 1,
//     OT_AND,
//     OT_PIPE,
// };

// enum e_io_type
// {
//     IO_REDIRECT_INPUT = 1,
//     IO_REDIRECT_OUTPUT,
//     IO_APPEND_OUTPUT,
//     IO_HEREDOC,
// };

// typedef struct s_cmd_arg
// {
//     char *name;
//     struct s_cmd_arg *next;
// } t_cmd_arg;

// //This holds command-related information

// typedef struct s_cmd_data {
//     t_cmd_arg *arg;  // Command and its arguments (e.g., "ls", "-l")
//     t_cmd_io *io;    // Input/output redirection data
// } t_cmd_data;

// //This represents operator nodes

// typedef struct s_op_data {
//     enum e_op_type type;  // Operator type (pipe, and, or, etc.)
//     t_node *left;         // Left-hand side of the operation (e.g., command before pipe)
//     t_node *right;        // Right-hand side (e.g., command after pipe)
// } t_op_data;

// //The core structure for each AST node, which can either be a command or an operator

// typedef struct s_node {
//     enum e_node_type type;  // Type of node (command or operator)
//     union u_node_data *data; // Data: either command or operator
// } t_node;

// //A union that holds either command data (t_cmd_data) or operator data (t_op_data)

// union u_node_data {
//     t_cmd_data *cmd_data;  // Command data
//     t_op_data *op_data;    // Operator data
// };

// execute the node and traverse the AST

// void execute_node(t_node *node, int nb_pipes)
// {
//     if (node == NULL)
//         return;
//     if (node->type == NT_OP)
//     {
//         execute_node(node->data->op_data->left, nb_pipes);
//         execute_node(node->data->op_data->right, nb_pipes);
//         // Handle the operator
//         // printf("--> %s\n", node->data->cmd_data->arg->name);
//         execute_operator(node->data->op_data, nb_pipes);
//     }
//     // else if (node->type == NT_CMD) {
//     //     // Handle command execution
//     //     execute_cmd(node->data->cmd_data);
//     //     //TODO:first thing execute the herdoc
//     // }
// }

// execute commande and handle input , output redirections

char *find_path(char *cmd, char *path)
{
    int i;

    i = 0;
    while (cmd[i] != ' ' && cmd[i])
        i++;
    if (cmd[i] != '\0')
        cmd = ft_substr(cmd, 0, i);
    path = ft_strjoin(path, cmd);
    if (access(path, X_OK) == 0)
        return (path);
    return (NULL);
}
char *get_env(void)
{
    char *PATH;
    char **t;

    PATH = getenv("PATH");
    t = ft_split(PATH, ':');
    PATH = ft_strjoin(t[6], "/");
    return (PATH);
}

int ft_lstsize(t_cmd_arg *lst)
{
    int size;

    size = 0;
    while (lst)
    {
        size++;
        lst = lst->next;
    }
    return (size);
}

char **create_arg(t_cmd_arg *list)
{
    char **arg;
    int size;
    int i;

    size = ft_lstsize(list);
    arg = malloc(sizeof(char *) * (size + 1));
    i = 0;
    while (i < size)
    {
        arg[i] = list->name;
        list = list->next;
        i++;
    }
    arg[i] = NULL;
    return (arg);
}

void execute_cmd(t_cmd_data *cmd_data)
{
    char *path;
    char **arg;
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (cmd_data->io != NULL)
            setup_io_redirection(cmd_data->io);
        // TODO:create function that get argement in an array from the list
        // TODO:check if the path is workin corectly
        path = get_env();
        path = find_path(cmd_data->arg->name, path);
        // printf("path--->%s\n", path);
        arg = create_arg(cmd_data->arg);
        // int i = 0;
        // while(arg[i])
        // {
        //     printf("I am here\n");
        //     printf("arg %d = %s\n",i, arg[i]);
        //     i++;
        // }
        if (execve(path, arg, NULL) == -1)
            printf("command not found: %s\n", cmd_data->arg->name);
    }
    waitpid(pid, &status, 0);
}

// handle operators pipe , or or and

// void execute_operator(t_op_data *op_data, int nb_pipes)
// {
//     /*
//         if I have a pipe operator I have to pipe first and fork for the child process to execute the left child where I have to close the read pipe if I have no input_redirection
//         then execute the left_node in the child process , in the parent process I have to close the write pipe and dup the read pipe
//         then execute recursivly the right_node
//         we have to execute the right_node recurcively to ensure that the other pipes and command gonna executes properly
//     */
//     if (op_data->type == OT_PIPE)
//     {
//         execute_pipe(op_data, nb_pipes);
//     }
// }

int calcule_pipes(t_node *node)
{
    int i;

    i = 0;
    if (!node)
        return (0);
    if (node->data->op_data->type == OT_PIPE)
        i++;
    return (i + calcule_pipes(node->data->op_data->right) + calcule_pipes(node->data->op_data->left));
}

void execute_pipe(t_op_data *op_data, int nb_pipes)
{
    int end_pip[2];
    int status;
    int prev_end[2];
    pid_t pid;
    t_node *node_left;
    t_node *node_right;

    prev_end[0] = -1;
    prev_end[1] = -1;
    node_left = op_data->left;
    if (op_data->left->type == NT_OP)
    {
        node_left = op_data->left->data->op_data->left;
        while (node_left->type == NT_OP)
            node_left = op_data->left;
    }
    node_right = op_data->right;
    if (op_data->left->type == NT_OP)
    {
        node_right = op_data->left->data->op_data->right;
        while (node_right->type == NT_OP)
            node_right = op_data->right;
    }
    while (nb_pipes >= 0)
    {
        if (nb_pipes > 0)
        {
            if (pipe(end_pip) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        pid = fork();
        if (pid == -1)
        {
            perror("fork:");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
            child_process(node_left, end_pip, prev_end, nb_pipes);
        set_prev_end(prev_end, end_pip);
        node_left = node_right;
        if (op_data->right->type == NT_OP)
        {
            node_right = op_data->right->data->op_data->left;
            while (node_right->type == NT_OP)
                node_right = op_data->left;
        }
        else if (op_data->right->type == NT_CMD)
            node_right = op_data->right;
        nb_pipes--;
    }
    while (waitpid(-1, &status, 0) > 0)
        ;
    close(end_pip[0]);
    close(end_pip[1]);
}

void child_process(t_node *node, int end[2], int prev_end[2], int nb_pipes)
{
    char **arg;
    char *path;

    if (prev_end[0] != -1)
    {
        close(prev_end[1]);
        if (dup2(prev_end[0], STDIN_FILENO) == -1)
        {
            perror("dup2 failed for prev_end[0] (STDIN)");
            exit(EXIT_FAILURE);
        }
        close(prev_end[0]);
    }
    if (node->data->cmd_data->io != NULL)
        setup_io_redirection(node->data->cmd_data->io);
    if (nb_pipes > 0)
    {
        close(end[0]);
        if (dup2(end[1], STDOUT_FILENO) == -1)
        {
            perror("dup2222222222222");
            exit(EXIT_FAILURE);
        }
        close(end[1]);
    }
    path = get_env();
    path = find_path(node->data->cmd_data->arg->name, path);
    arg = create_arg(node->data->cmd_data->arg);
    if (execve(path, arg, NULL) == -1)
        printf("command not found: %s\n", node->data->cmd_data->arg->name);
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

// //setup io rederction
// typedef struct s_cmd_io
// {
//     char *file;
//     enum e_io_type type;
//     struct s_cmd_io *next;
// } t_cmd_io;

void redirect_input(char *file)
{
    int fd;

    // check if the flags is correct
    fd = open(file, O_RDWR | O_CREAT, 0644);
    if (fd == -1)
        exit(EXIT_FAILURE);
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void redirect_output(char *file)
{
    int fd;

    // check if the flags is correct
    fd = open(file, O_RDWR | O_CREAT, 0644);
    if (fd == -1)
        exit(EXIT_FAILURE);
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void append_output(char *file)
{
    int fd;

    fd = open(file, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
        exit(EXIT_FAILURE);
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}
void setup_io_redirection(t_cmd_io *cmd_io)
{
    while (cmd_io)
    {
        if (cmd_io->type == IO_REDIRECT_INPUT)
            redirect_input(cmd_io->file);
        else if (cmd_io->type == IO_REDIRECT_OUTPUT)
            redirect_output(cmd_io->file);
        else if (cmd_io->type == IO_APPEND_OUTPUT)
            append_output(cmd_io->file);
        // else
        //     handle_herdoc(cmd_io);//TODO:impliment the function
        cmd_io = cmd_io->next;
    }
}

int main()
{
    t_ast *ast;
    int nb_pipes;
    int end[2];

    while (1)
    {
        char *line = readline(">> ");
        ast = ast_parse(line);
        // printf("----->%s\n", ast->root->data->cmd_data->arg->name);
        // if(ast->root->data->cmd_data->arg->next)
        //     printf("----->%s\n", ast->root->data->cmd_data->arg->next->name);
        // execute_cmd(ast->root->data->cmd_data);
        if (ast->root->type == NT_CMD)
            execute_cmd(ast->root->data->cmd_data);
        else if (ast->root->data->op_data->type == OT_PIPE)
        {
            nb_pipes = calcule_pipes(ast->root);
            execute_pipe(ast->root->data->op_data, nb_pipes);
        }
    }
    return (0);
}
