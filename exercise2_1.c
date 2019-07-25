#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 * Create this process tree:
 * A-+-B---D
 *   `-C
 */
void fork_procs(void)
{
        pid_t pid_B;
        pid_t pid_C;
        pid_t pid_D;
        int status_B;
        int status_C;
        int status_D;
        /*
         * initial process is A.
         */
        fprintf(stderr, "Parent, PID = %ld: Creating the B ...\n", (long)getpid());
        change_pname("A");
        pid_B = fork();/*A creates B*/
        if (pid_B < 0) {
                perror("Error at fork that creates B\n");
                exit(1);
        }
        if (pid_B == 0) {
                /* B*/
                change_pname("B");
                fprintf(stderr, "Parent, PID = %ld: Creating the D ...\n", (long)getpid());
                pid_D=fork();/*B is not a terminating process so it creates D*/
                if (pid_D < 0) {
                        perror("Error at fork that creates C\n");
                        exit(1);
                }
                if (pid_D == 0) {
                        /*D*/
                        change_pname("D");
                        printf("D: Sleeping...\n");
                        sleep(SLEEP_PROC_SEC);/*D has no children so it sleeps*/
                        printf("D: Exiting...\n");
                        exit(13);
                }
                printf("B: Waiting for child D to terminate...\n");
                pid_D = wait(&status_D);/*B waits D to terminate*/
                explain_wait_status(pid_D, status_D);
                printf("B: Exiting...\n");
                exit(19);
                }
        fprintf(stderr, "Parent, PID = %ld: Creating the C ...\n", (long)getpid());
        pid_C=fork();/*A creates C*/
        if (pid_C < 0) {
                perror("Error at fork that creates C\n");
                exit(1);
        }
        if (pid_C == 0) {
                /*C*/
                change_pname("C");
                printf("C: Sleeping...\n");
                sleep(SLEEP_PROC_SEC);/*C has no children so it sleeps*/
                printf("C: Exiting...\n");
                exit(17);
        }

        /*First it A waits B to terminate then C*/
        printf("A: Waiting for child B to terminate...\n");
        /*pid_B = */waitpid(pid_B,&status_B,0);
        explain_wait_status(pid_B, status_B);
        printf("A: Waiting for child C to terminate...\n");
        pid_C = wait(&status_C);
        explain_wait_status(pid_C, status_C);

        printf("A: Exiting...\n");
        exit(16);
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */
int main(void)
{
        pid_t pid;
        int status;

        /* Fork root of process tree */
        fprintf(stderr, "Parent, PID = %ld: Creating the root A of process tree ...\n", (long)getpid());
        pid = fork();
        if (pid < 0) {
                perror("main: fork");
                exit(1);
        }
        if (pid == 0) {
                /* Child */
                fork_procs();
                exit(1);
        }
        /*Father*/

        sleep(SLEEP_TREE_SEC);

        /* Print the process tree root at pid */
        show_pstree(getpid());


        /* Wait for the root of the process tree to terminate */
        printf("Waiting for the root of the process tree A to terminate\n");
        pid = wait(&status);
        explain_wait_status(pid, status);
        fflush(stderr);

        return 0;
}
