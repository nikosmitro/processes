#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

pid_t fork_procs4(struct tree_node *node, int fd_father[2])
{       /*This function creates process trees that make arithmetic calculations.Each process of the tree if its not a leaf process,
          it has 2 children and it comminicates with them via pipes.The leaf processes go to sleep and their father waits for them.*/

        int fd1[2], fd2[2];
        int status;
        pid_t pid, *pid_child;
        int num1, num2;
        int nr_children;
        char *name;
        nr_children=node->nr_children;
        name=node->name;

        pid = fork();
        if (pid<0){/*Error*/
                printf("%s :",node->name);
                perror("fork");
                exit(-1);
        }
        if (pid==0){
                change_pname(name);     /*Change the process name*/
                printf("%s : Created \n",name); /*Message "created"*/
                if (nr_children==0){    /*it's a leaf*/
                        num1 = atoi(name); /*converts the char to int so we can make calculations*/
                        printf("%s with PID = %ld sends %d to pipe \n", name,(long)getpid(),num1);
                        if (write(fd_father[1],&num1,sizeof(num1))<0){  /*sends the number (name) to father*/
                                perror("write pipe");
                                exit(2);
                        }
                        sleep(SLEEP_PROC_SEC);  /*its a leaf process so iut sleeps*/
                        printf("%s: Exiting...\n", name);       /*then sleeps because we want to see the tree*/
                        exit(8);
                }
                pid_child = (pid_t *)malloc(2*sizeof(pid_t));/*make room fot the array of the pids of the children*/
                if (pipe(fd1)<0){
                        perror ("pipe");     /*create a pipe for the first child*/
                        exit(4);
                }
                pid_child[0]=fork_procs4(node->children, fd1);  /*recursion in order to create the first  child*/
                if (pipe(fd2)<0) {
                        perror ("pipe");    /*create a second pipe for the second child*/
                        exit(4);
                }
                pid_child[1]=fork_procs4(node->children+1, fd2);        /*recursion in order to create the second child */
                /*printf("%s: Waiting...\n", name);*/   /*because procedure is father of 2 other procedures as defined from the exercise*/
                printf("Parent %s with PID=%ld : Created child with PID = %ld, waiting for it to terminate...\n",name,(long)getpid(),(long)pid_child[0]);
                waitpid(pid_child[0],&status,0);        /*Waiting for the first  child to be terminated*/
                explain_wait_status(pid_child[0], status);
                printf("Parent %s with PID=%ld : Created child with PID = %ld, waiting for it to terminate...\n",name,(long)getpid(),(long)pid_child[1]);
                pid_child[1] = wait(&status);        /*Waiting for the second  child to be terminated*/
                explain_wait_status(pid_child[1], status);
                close(fd1[1]);  /* close up the output side (write) of the first  pipe */
                if (read(fd1[0],&num1,sizeof(num1))<0){  /*read from pipe1 the value of the first child*/
                        perror("read pipe");
                        exit(2);
                }
                close(fd1[0]);   /* close up the input side (read) of the first  pipe */
                close(fd2[1]);    /* close up the output side (write)  of the second  pipe */
                if (read(fd2[0],&num2,sizeof(num2))<0){   /*read from pipe2 the value of the second child*/
                        perror("read pipe");
                        exit(2);
                }
                close(fd2[0]);  /* close up the input side (read) of the second  pipe */
                printf("%s with PID = %ld reads %d and %d from pipe \n" ,name,(long)getpid(),num1,num2);
                printf("Process with PID = %ld : calculates %d %s %d \n",(long)getpid(), num1,name, num2);
                switch(node->name[0]){  /*choose operation*/
                        case '+':
                                num1 = num1+num2;
                                break;
                        case '-':
                                num1 = num1-num2;
                                break;
                        case '/':
                                num1 = num1/num2;
                                break;
                        case '*':
                                num1 = num1*num2;
                                break;
                }
                printf("%s: %d sends %d to pipe \n",name,getpid(),num1);
                if (write(fd_father[1],&num1,sizeof(num1))<0){  /*send the result to the pipe of the father*/
                        perror("write pipe");
                        exit(2);
                }
                     printf("%s: Exiting...\n",name);
                exit(8);
        }
        return pid;
}

int main(int argc, char *argv[])
{
        int fd[2]; /*pipe*/
        int num1;
        pid_t pid;
        int status;
        struct tree_node *root;

        if (argc != 2) {/*We need 2 arguments*/
        fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
        exit(1);
        }
        root = get_tree_from_file(argv[1]);     /*get tree from file and and also the start of struct  (tree_node)*/
        print_tree(root);   /*and print it*/
        if (pipe(fd)<0)
        {
                perror ("pipe");
                exit(3);
        }
        pid = fork_procs4(root,fd);     /*returns pid of root (the first call of the function)*/
        sleep(SLEEP_TREE_SEC);  /*sleep until all procedures of tree created*/
        show_pstree(pid);       /* Print the process tree root at pid */
        pid = wait(&status);    /* Wait for the root of the process tree to terminate */
        explain_wait_status(pid, status);
        close(fd[1]);  /*close up the output side (write) of the pipe*/
        if (read(fd[0],&num1,sizeof(num1))<0 ){  /*read the result from pipe (from tree's root)*/
                perror("read pipe");
                exit(2);
        }
        close(fd[0]); /*close up the input side (read) of the pipe*/
        printf(" \nTHE RESULT IS : %d \n",num1);
        return 0;
}
           
