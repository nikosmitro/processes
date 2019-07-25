Four progrmmes that focus on process management and process communication.

Program - Exercise 1.

  Creates a standard process tree. The tree is printed so the user can see it. Each leaf process sleeps for 10 sec and each parent         process wait for its children processes to terminate. Each process prints a status message (starting,terminating...). To discriminate the processes each process terminates with an unique ID number.
  
  
Program - Exercise 2.

  Creates an arbitrary process tree based on an input file. The input file describes the tree node by node starting from the root.
  For each node is given its name, te number of its children and their names. The program is based on the recursive function fork_procs2()   which is call for each node of the tree.


Program - Exercise 3.

  Expansion of program 2. The processes are control by signals, so that they print their messages by deep first. Each process creates its   children and suspend its operation until it get the signal SIGCONT. Once it gets it, it enables its children one by one. The program is   based on recirsion.  
 
 
Program - Exercise 4.

  Expansion of program 2. The process tree computes an arithmetic operations. Each non leaf procees of the tree represents an arithmetic     operator "+", "-", "*", "/". Each leaf process represents a number. For the communication of the processes Unix pipes are used.
  
  
