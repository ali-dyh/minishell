# Execution Section

I.Process Creation and Management:
    https://blogs.30dayscoding.com/blogs/c/real-world-c-applications/system-programming/process-management/

    -> Process image : 
    Process image is an executable file required while executing the program. This image usually contains the following sections −

    Code segment or text segment
    Data segment
    Stack segment
    Heap segment

    -> Fork() system call:
    https://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/create.html
    
    The fork() system call in Unix-like operating systems is used to create a new process by duplicating the calling (parent) process. It returns a value that can be used to distinguish between the parent process and the newly created child process.

    Here's how the return value of fork() works:

    1. Return Value in the Parent Process:
        Positive Number:
        If fork() returns a positive number in the parent process, this value is the Process ID (PID) of the newly created child process.
        The parent process can use this PID to manage the child process, for instance, to wait for it to finish using wait() or to send signals to it.
        Example: If fork() returns 1234 in the parent process, then 1234 is the PID of the child process.
    2. Return Value in the Child Process:
        0:
        If fork() returns 0, it means the code is running in the child process.
        The child process is an almost exact copy of the parent process, except for the return value of fork() and a few other differences (like the PID).
    3. Return Value on Failure:
        -1:
        If fork() returns -1, it means the fork operation failed, and no child process was created.
        This can happen due to various reasons, such as reaching the system's limit on the number of processes or insufficient memory.
-> execve() system call :
    https://docs.oracle.com/cd/E19048-01/chorus5/806-7016/6jftugggq/index.html#:~:text=5FEA)%20for%20details.-,DESCRIPTION,is%20an%20executable%20object%20file.

    How execve Works :
        When execve is called, the current process is replaced by the new program specified by pathname. This means that:
        The current process's memory (code, data, stack, heap, etc.) is entirely replaced by the new program's memory.
        The process ID (PID) remains the same, but everything else about the process is changed to reflect the new program.
        Any open file descriptors remain open unless the new program explicitly closes them.
        The new program starts executing from its main function, using argv[] as its arguments and envp[] as its environment.

-> waitpid() :
    https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-waitpid-wait-specific-child-process-end
    Process Groups:

    Groupe id:
        In Unix-like operating systems, processes can be grouped into process groups. Each process group has a unique Process Group ID (PGID).
        By default, when a process is created (using fork()), it inherits the process group ID of its parent. However, processes can change their process group ID, either forming new groups or joining existing ones.
    
    Behavior of waitpid() with pid = 0:
        When you call waitpid(0, &status, options), you're asking the system to wait for any child process within the same process group as the caller.
        In simpler terms, if pid is set to 0, waitpid() will wait for any child process that belongs to the same process group as the process that called waitpid().

    Why is this Useful?
    Job Control:
        This feature is particularly useful in scenarios involving job control in the shell. For example, when a shell runs a pipeline of commands (e.g., cmd1 | cmd2 | cmd3), all these commands are typically placed in the same process group.
        The shell might then use waitpid(0, &status, 0) to wait for any of the processes in the pipeline to finish.
    Process Management:
        It allows a process to manage its child processes more effectively, especially when dealing with related processes that need to be monitored or terminated as a group.
    waitpid options :
       (NB)=> When you pass 0 as the options argument to waitpid, it means that you're using the default behavior of waitpid.

II. File Descriptors and Redirections : 
    File Descriptors :
    https://medium.com/@dhar.ishan04/here-is-all-you-need-to-know-about-file-descriptors-in-linux-d93f05166026#:~:text=File%20descriptors%20are%20simply%20non,interactions%20with%20various%20governmental%20services.
    