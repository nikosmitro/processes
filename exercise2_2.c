#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  15
#define SLEEP_TREE_SEC  3


 void fork_procs2(struct tree_node *node)
{
        /*Creates a process tree based on an input file that its informations have been written in structs tree_node
         * the leaf processes(no children) go to sleep and the processes with children waits for them to exit*/

        char *name;
        unsigned nr_children;
        int i;
        pid_t pid;
        int status;

        name=node->name;/*take the name of the process based on the file*/
        change_pname(name);
        nr_children=node->nr_children;/*take the number of children of the process based on the file*/
        if(nr_children==0)/*if the process is leaf it goes to sleep*/
        {
                printf("%s:Sleeping....\n",name);
                sleep(SLEEP_PROC_SEC);
        }
        else/*if is not a leaf it creates nr_children children*/
        {
                for (i=0; i<nr_children; i++)
                {
                        fprintf(stderr,"Parent, PID = %ld: Creating the %s ...\n", (long)getpid(),node->children[i].name);
                        pid=fork();
                        if(pid<0) {
                                fprintf(stderr,"Error at fork of %s\n",name);
                                exit(1);
                        }
                        if(pid==0) {/*child*/      /*Recursion in order to create the tree*/
                                fork_procs2(&node->children[i]);
                                exit(1);
                        }
                }
                printf("%s :Waiting...\n",name);
                for (i=0; i<nr_children; i++)
                {      /*after the children are created the father waits for them to exit*/
                        pid=wait(&status);
                        explain_wait_status(pid,status);
                }
        }
        printf("%s:Exiting...\n",name);
        exit(8);
}


int main(int argc, char *argv[])
{
        struct tree_node *root;
        pid_t pid;
        int status;

        /*We must have 2 arguments*/
        if (argc != 2) {
                fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
                exit(1);
        }
        /*Get the tree from file and then Print it*/
        root = get_tree_from_file(argv[1]);
        print_tree(root);
        /* Fork root of process tree */
        fprintf( stderr,"Parent, PID = %ld: Creating the root %s of process tree ...\n", (long)getpid(),root->name);
        pid = fork();
        if (pid < 0) {
                perror("main: fork");
                exit(1);
        }
        if (pid == 0) {
                        /* Child */
                fork_procs2(root);/*call the recursive function to start creating the tree*/
                exit(1);
        }
        sleep(SLEEP_TREE_SEC);/*tree sleeping*/

        /* Print the process tree root at pid */
        show_pstree(pid);

        /* Wait for the root of the process tree to terminate */
        printf("Waiting for the root of the process tree %s to terminate\n",root->name);
        pid = wait(&status);
        explain_wait_status(pid, status);
        fflush(stderr);


        return 0;
}

