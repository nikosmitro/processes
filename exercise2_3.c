#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc-common.h"
#include "tree.h"

pid_t fork_procs3(struct tree_node *node)
{
        /*Creates A PROCESS TREE based on a file that its information is stored at a struct tree_node
         *  and sychronize the processes in the way that they start and terminate by DFS metod*/

        int i,status;
        pid_t pid, *pid_child;
        int nr_children;
        char *name;

        name=node->name;/*get the name of the process based on the file*/
        nr_children=node->nr_children;/*get the number of the children of the process based on the file*/
        pid = fork();
        if (pid<0){  /*Error*/
                printf("Error with the fork of %s\n",name);
                exit(1);
        }
        if (pid==0){
                printf("Name %s, PID = %ld ,starting... \n",name,(long)getpid());   /*message for starting*/
                change_pname(node->name);       /*change process name*/
                pid_child = (pid_t *)malloc((nr_children)*sizeof(pid_t));  /*make room for the array that contains the pids of the children of the process*/
                for (i=0; i<nr_children; i++){
                        pid_child[i] = fork_procs3(node->children+i); /*store in the array the pids of children*/
                        wait_for_ready_children(1);  /*Every process as a father waits for the child to be ready (=stopped by SIGSTOP) before create  the next child
                                                       in order to have DFS*/ /*if he created all his children first and then wait all of them we wouldnt have DFS*/
                }
                raise(SIGSTOP); /*then stops until SIGCONT*/
                printf("Name %s, PID = %ld is awake\n",name,(long)getpid()); /*gets the SIGCONT signal and becomes awake*/
                for (i=0; i<nr_children; i++) {
                        pid = pid_child[i];
                        kill(pid,SIGCONT);   /*sends a SIGCONT signal to every child*/
                        /*then waits for the child to terminate*/
                        printf("%s with PID=%ld is waiting for %s with PID=%ld to terminate\n",node->name,(long)getpid(),node->children[i].name,(long)pid);
                        waitpid(pid,&status,0);
                        explain_wait_status(pid, status);
                }
                /*Exiting...*/
                printf("Name %s, PID = %ld, exiting... \n",name,(long)getpid());
                exit(8);
        }
        return pid;  /*returns the pid of the process so we can store them later in the array*/
}

int main(int argc, char *argv[])
{
        pid_t pid;
        int status;
        struct tree_node *root;
        if (argc < 2) { /*we need 2 arguments*/
                fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
                exit(1);
        }

        root = get_tree_from_file(argv[1]);     /*get tree from file and the start of the struct (tree_node)*/
        print_tree(root);
        pid = fork_procs3(root);        /*returns pid of root*/
        wait_for_ready_children(1);     /*wait for root process to be ready(=stopped by SIGSTOP signal)*/
        show_pstree(pid);       /* Print the process tree root at pid */
        kill(pid,SIGCONT);      /*send SIGCONT to root*/
        pid = wait(&status);    /* Wait for the root of the process tree to terminate */
        explain_wait_status(pid, status);
        return 0;
}
