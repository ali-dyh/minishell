/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/19 12:16:43 by marvin            #+#    #+#             */
/*   Updated: 2024/10/19 12:16:43 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include<stdlib.h>
#include<fcntl.h>

enum e_syntax_error
{
    NO_SYN_ERR,
    UNEXPECTED_EOF,
    UNEXPECTED_TOKEN,
    WTF // malloc failure
};

enum e_node_type
{
    NT_CMD = 1,
    NT_OP
};

enum e_op_type
{
    OT_OR = 1,
    OT_AND,
    OT_PIPE,
};

enum e_io_type
{
    IO_REDIRECT_INPUT = 1,
    IO_REDIRECT_OUTPUT,
    IO_APPEND_OUTPUT,
    IO_HEREDOC,
};

typedef struct s_cmd_arg
{
    char *name;
    struct s_cmd_arg *next;
} t_cmd_arg;

//This holds command-related information

typedef struct s_cmd_data {
    t_cmd_arg *arg;  // Command and its arguments (e.g., "ls", "-l")
    t_cmd_io *io;    // Input/output redirection data
} t_cmd_data;


//This represents operator nodes

typedef struct s_op_data {
    enum e_op_type type;  // Operator type (pipe, and, or, etc.)
    t_node *left;         // Left-hand side of the operation (e.g., command before pipe)
    t_node *right;        // Right-hand side (e.g., command after pipe)
} t_op_data;

//The core structure for each AST node, which can either be a command or an operator

typedef struct s_node {
    enum e_node_type type;  // Type of node (command or operator)
    union u_node_data *data; // Data: either command or operator
} t_node;

//A union that holds either command data (t_cmd_data) or operator data (t_op_data)

union u_node_data {
    t_cmd_data *cmd_data;  // Command data
    t_op_data *op_data;    // Operator data
};

//execute the node and traverse the AST

void execute_node(t_node *node)
{

    if (node == NULL) 
        return;
    if (node->type == NT_OP)
    {
        execute_node(node->data->op_data->left);
        execute_node(node->data->op_data->right);
        // Handle the operator 
        execute_operator(node->data->op_data);
    } else if (node->type == NT_CMD) {
        // Handle command execution
        execute_cmd(node->data->cmd_data);
        //TODO:first thing execute the herdoc 
    }
}

//execute commande and handle input , output redirections

void execute_cmd(t_cmd_data *cmd_data)
{
    // char *path;
    // char **arg;
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
        if (cmd_data->io != NULL) {
            // Handle I/O redirection (if any)
            setup_io_redirection(cmd_data->io);
        }
        //TODO:create function that get argement in an array from the list
        //TODO:check if the path is workin corectly
        // path = get_env("PATH");
        // path = find_path(node->data->cmd_data->arg->name, path);
        // arg = create_arg(node->data->cmd_data->arg->name, path);
        if(execve(cmd_data->arg->argv[0], cmd_data->arg->argv, NULL) == -1)
            printf("command not found: %s\n", node->data->cmd_data->arg->name);
    }
    waitpid(pid, &status, 0);
}

// handle operators pipe , or or and

void execute_operator(t_op_data *op_data)
{
    /*
        if I have a pipe operator I have to pipe first and fork for the child process to execute the left child where I have to close the read pipe if I have no input_redirection
        then execute the left_node in the child process , in the parent process I have to close the write pipe and dup the read pipe 
        then execute recursivly the right_node 
        we have to execute the right_node recurcively to ensure that the other pipes and command gonna executes properly
    */
    if(op_data->type == OT_PIPE)
        execute_pipe(op_data);
        
}

void execute_pipe(t_op_data *op_data)
{
    int pipe_end[2];
    t_pid pid;
    int status;

    if(pipe(pipe_end) == -1)
    {
        perror("pipe:");
    }
    pid = fork();
    if(pid == 0)
    {
        close(pipe_end[0]);
        dup2(pipe_end[1], STDOUT_FILENO);
        close(pipe_end[1]);
        execute_cmd(op_data->left);
    }
    else if(pid > 0)
    {
        close(pipe_end[1]);
        dup2(pipe_end[0], STDIN_FILENO);
        close(pipe_end[0]);
        execute_pipe(op_data->right);
    }
    wait_pid(pid, &status, 0);
}


//setup io rederction 
typedef struct s_cmd_io
{
    char *file;
    enum e_io_type type;
    struct s_cmd_io *next;
} t_cmd_io;

void setup_io_redirection(t_cmd_io *cmd_io)
{
    while(cmd_io)
    {
        if (cmd_io->type == IO_REDIRECT_INPUT)
            redirect_input(cmd_io);
        else if(cmd_io->type == IO_REDIRECT_OUTPUT)
            redirect_output(cmd_io);
        else if(cmd_io->type == IO_APPEND_OUTPUT)
            append_output(cmd_io);
        else
            handle_herdoc(cmd_io);//TODO:impliment the function
        cmd_io = t_cmd_io->next;
    }
}

void redirect_input(char *file)
{
    int fd ;

    //check if the flags is correct
    fd = open(file, O_WRONLY | O_CREAT);
    if(fd == -1)
        exit(EXIT_FAILURE);
    if(dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void redirect_output(char *file)
{
    int fd ;

    //check if the flags is correct
    fd = open(cmd_io->file, O_WRONLY | O_CREAT);
    if(fd == -1)
        exit(EXIT_FAILURE);
    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void append_output(char *file)
{
    int fd ;

    fd = open(cmd_io->file, O_WRONLY | O_CREAT | O_APPEND);
    if(fd == -1)
        exit(EXIT_FAILURE);
    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}
