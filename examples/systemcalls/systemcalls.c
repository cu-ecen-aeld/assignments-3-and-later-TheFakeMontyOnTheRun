#include "systemcalls.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
  int status = system(cmd);
  return (WIFEXITED(status) && WEXITSTATUS(status) == 0) || (cmd == NULL && status == 0);
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    fflush(stdout);
    int pid = fork();
    if (!pid) {
      puts("*** running ***");
      for ( i = 0; i < count; ++i) {
	puts(command[i]);
      }
      puts("****DONE****");
      int status = execv(command[0], command + 1);
      printf("Got %d\n", status);
      puts("FAILED");
      return false;
    } else {
      int status;
      waitpid(pid, &status, 0);
      printf("Got from child: %d\n", status);
      if (status == -1) {
	puts("FAILURE ACK");
	return false;
      }
    }

    va_end(args);
    puts("ALL GOOD!");
    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;


    // Source - https://stackoverflow.com/a
    // Posted by tmyklebu, modified by community. See post 'Timeline' for change history
    // Retrieved 2025-11-10, License - CC BY-SA 3.0
    
    int kidpid;
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); abort(); }

    fflush(stdout);
    switch (kidpid = fork()) {
    case -1: perror("fork"); abort();
    case 0:
      if (dup2(fd, 1) < 0) { perror("dup2"); abort(); }
      close(fd);
      execv(command[0], command);
      va_end(args);
      return false;
    default: {
      int status;
      waitpid(kidpid, &status, 0);
      close(fd);
      if (status == -1) {
	return false;
      }
    }
    }
    

    
    va_end(args);
    
    return true;
}
