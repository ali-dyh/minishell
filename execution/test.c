/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cboujrar <cboujrar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/19 12:16:43 by marvin            #+#    #+#             */
/*   Updated: 2024/10/25 15:38:49 by cboujrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../parsing/parser.h"
#include "cJSON/cJSON.h"

void setup_io_redirection(t_cmd_io *cmd_io);
int  execute_pipe(t_op_data *op_data, int nb_pipes);
int execute_operator(t_op_data *op_data, int nb_pipes, int status);
int execute_cmd(t_cmd_data *cmd_data);
char **ft_split(const char *s, char c);
size_t ft_strlcpy(char *dst, char *src, int size);
char *ft_substr(char *s, int start, int len);
char *ft_strjoin(char const *s1, char const *s2);
int calcule_pipes(t_node *node);
void child_process(t_node *node, int end[2], int prev_end[2], int nb_pipes);
void set_prev_end(int prev_end[2], int end[2]);
int execute_and(t_op_data *op_data, int status, int nb_pipes);
int execute_or(t_op_data *op_data, int status, int nb_pipes);



t_node *find_parent(t_node *root, t_node *target) 
{
    t_node *left_search;
    
    if (root == NULL || target == NULL || root == target) {
        return NULL;
    }

    if(root->type == NT_OP)
    {
        if (root->data->op_data->left == target || root->data->op_data->right == target) {
            return root;
        }  
    }
    else
        left_search = NULL;

    left_search = find_parent(root->data->op_data->left, target);
    if (left_search != NULL) {
        return left_search; 
    }
    return find_parent(root->data->op_data->right, target);
}

// execute the node and traverse the AST

int execute_node(t_node *root, t_node *node, int nb_pipes,int status)
{
    t_node *parent;
      
    if (node == NULL)
        return(status);

    // status = -1;
    if(node->type == NT_OP)
    {
        if(node->data->op_data->type == OT_PIPE)
        {
            status = execute_node(root, node->data->op_data->left, nb_pipes, status);
            status = execute_node(root, node->data->op_data->right, nb_pipes, status);
            // printf("I am here from pipes \n");
            parent = find_parent(root, node);
            if(!parent ||  parent->data->op_data->type != OT_PIPE)
                status = execute_operator(node->data->op_data, nb_pipes, status);
        }
        else
        {
            status = execute_node(root ,node->data->op_data->left, nb_pipes, status);
            if((node->data->op_data->type == OT_AND && (status == 0 || status == -1)) || ((node->data->op_data->type == OT_OR &&  (status != 0 || status == -1))))
            {
                if(node->data->op_data->right->data->op_data->type != OT_PIPE)
                {
                    status = execute_node(root ,node->data->op_data->right, nb_pipes, status);
                    // printf("I am here from execute right node  %s\n", node->data->op_data->right->data->op_data->left->data->cmd_data->arg->name);
                }
                status = execute_operator(node->data->op_data, nb_pipes, status);  
            }
        }
    }
    else
        status = -1;
    // else if (node->type == NT_CMD) {
    //     // Handle command execution
    //     status = execute_cmd(node->data->cmd_data);
    //     //TODO:first thing execute the herdoc
    // }
    return(status);
}

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

int execute_cmd(t_cmd_data *cmd_data)
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
        path = get_env();
        path = find_path(cmd_data->arg->name, path);
        arg = create_arg(cmd_data->arg);
        if (execve(path, arg, NULL) == -1)
            printf("command not found: %s\n", cmd_data->arg->name);
    }
    waitpid(pid, &status, 0);
    return (status);
}


int execute_operator(t_op_data *op_data, int nb_pipes, int status)
{
    if (op_data->type == OT_PIPE)
        status = execute_pipe(op_data, nb_pipes);
    else if (op_data->type == OT_AND)
        status = execute_and(op_data, status, nb_pipes);
    else if (op_data->type == OT_OR)
        status = execute_or(op_data, status, nb_pipes);
    return (status);
}

// handle operators pipe
int calcule_pipes(t_node *node)
{
    int i;

    i = 0;
    if (!node)
        return (0);
    if (node->type == NT_OP)
    {
        if (node->data->op_data->type == OT_PIPE)
            i++;
        i = i + calcule_pipes(node->data->op_data->right);
        i = i + calcule_pipes(node->data->op_data->left);
    }
    return (i);
}

int  execute_pipe(t_op_data *op_data, int nb_pipes)
{
    int end_pip[2];
    int status;
    int prev_end[2];
    pid_t pid;
    t_node *node_left;
    t_node *node_right;
    int check;

    prev_end[0] = -1;
    prev_end[1] = -1;

    node_left = op_data->left;
    if (node_left->type == NT_OP)
    {
        while (node_left->type == NT_OP)
            node_left = node_left->data->op_data->left;
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
        nb_pipes--;
        check = nb_pipes;
        if (op_data->left->type == NT_OP)
        {
            node_right = op_data->right;
            node_left = op_data->left;
            while (node_left->type == NT_OP && check > 0)
            {
                node_right = node_left->data->op_data->right;
                node_left = node_left->data->op_data->left;
                check--;
            }
        }
        else if (op_data->right->type == NT_CMD)
            node_right = op_data->right;
        node_left = node_right;
    }
    while (waitpid(-1, &status, 0) > 0)
        ;
    close(end_pip[0]);
    close(end_pip[1]);
    // printf("status for pipe ---> %d\n", status);
    return(status);
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

// handle operator and

int execute_and(t_op_data *op_data, int status, int nb_pipes)
{
    if ((status == -1 || !status) && op_data->left->type == NT_CMD)
        status = execute_cmd(op_data->left->data->cmd_data);
    if (!status &&  op_data->right->type == NT_CMD)
        status = execute_cmd(op_data->right->data->cmd_data);
    else if(!status && op_data->right->type == NT_OP)
        status = execute_operator(op_data->right->data->op_data, nb_pipes,  status);
    return (status);
}

// handle operator or

int execute_or(t_op_data *op_data, int status, int nb_pipes)
{
    if ((status == -1 || status) && op_data->left->type == NT_CMD)
        status = execute_cmd(op_data->left->data->cmd_data);

    if (status != 0)
        status = execute_cmd(op_data->right->data->cmd_data);
    else if(status != 0 && op_data->right->type == NT_OP)
        status = execute_operator(op_data->right->data->op_data,nb_pipes,  status);
    return (status);
}


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

//////////////////////////////////////////////////////////////////////////////////////////////////

char *print_node_type(enum e_node_type node_type)
{
    switch (node_type)
    {
    case NT_CMD:
        return "CMD";
    default:
        return "OP";
    }
}

char *print_io_type(enum e_io_type io_type)
{
    switch (io_type)
    {
    case IO_REDIRECT_INPUT:
        return "REDIRECT INPUT";
    case IO_REDIRECT_OUTPUT:
        return "REDIRECT OUTPUT";
    case IO_APPEND_OUTPUT:
        return "APPEND OUTPUT";
    default:
        return "HEREDOC";
    }
}

char *print_op_type(enum e_op_type op_type)
{
    switch (op_type)
    {
    case OT_AND:
        return "AND";
    case OT_OR:
        return "OR";
    default:
        return "PIPE";
    }
}

cJSON *cmd_to_json(t_node *node)
{
    t_cmd_arg *arg = node->data->cmd_data->arg;
    t_cmd_io *io = node->data->cmd_data->io;
    cJSON *nodeObj = cJSON_CreateObject();
    cJSON *argArr = cJSON_CreateArray();
    cJSON *ioArr = cJSON_CreateArray();
    cJSON *ioObj;
    cJSON_AddStringToObject(nodeObj, "node_type", print_node_type(node->type));
    while (arg || io)
    {
        if (arg)
        {
            cJSON_AddItemToArray(argArr, cJSON_CreateString(arg->name));
            arg = arg->next;
        }
        if (io)
        {
            ioObj = cJSON_CreateObject();
            cJSON_AddStringToObject(ioObj, "io_type", print_io_type(io->type));
            cJSON_AddStringToObject(ioObj, "file_name", io->file);
            cJSON_AddItemToArray(ioArr, ioObj);
            io = io->next;
        }
    }
    cJSON_AddItemToObject(nodeObj, "args", argArr);
    cJSON_AddItemToObject(nodeObj, "ios", ioArr);
    return nodeObj;
}

cJSON *ast_to_json(t_node *node)
{
    if (node->type == NT_OP)
    {
        cJSON *nodeObj = cJSON_CreateObject();
        cJSON_AddStringToObject(nodeObj, "node_type", print_node_type(node->type));
        cJSON_AddStringToObject(nodeObj, "op_type", print_op_type(node->data->op_data->type));
        cJSON_AddItemToObject(nodeObj, "left_node", ast_to_json(node->data->op_data->left));
        cJSON_AddItemToObject(nodeObj, "right_node", ast_to_json(node->data->op_data->right));
        return nodeObj;
    }
    return cmd_to_json(node);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    t_ast *ast;
    int nb_pipes;
    int end[2];
    int status;

    while (1)
    {
        char *line = readline(">> ");
        ast = ast_parse(line);
        // cJSON *json = ast_to_json(ast->root);
        // char *jsonStr = cJSON_Print(json);
        // printf("%s\n", jsonStr);
        nb_pipes = calcule_pipes(ast->root);
        status = -1;
        // t_node *parent = find_parent(ast->root, ast->root->data->op_data->left);
        // printf("-->%s\n", parent->data->op_data->left->data->cmd_data->arg->name);
        // cJSON *node = ast_to_json(parent);
        // char *node_parent = cJSON_Print(node);
        // printf("%s\n", node_parent);
        if (ast->root->type == NT_CMD)
            status = execute_cmd(ast->root->data->cmd_data);
        else if(ast->root->type == NT_OP)
            status = execute_node(ast->root ,ast->root, nb_pipes, status);
        else if (ast->root->data->op_data->type == OT_PIPE)
        {
            // printf("--->%d\n", nb_pipes);
            execute_pipe(ast->root->data->op_data, nb_pipes);
        }
        // else if (ast->root->data->op_data->type == OT_AND)
        //     execute_and(ast->root->data->op_data);
        // else if (ast->root->data->op_data->type == OT_OR)
        //     execute_and(ast->root->data->op_data);
        // cJSON_free(jsonStr);
        // cJSON_Delete(json); 
        // cJSON_free(node_parent);
        // cJSON_Delete(node);
    }
    return (0);
}
