/* second pipe example from Haviland */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include<string.h>
#include <sys/wait.h>
#include <errno.h>

#define MSGSIZE 64

char *msg1 = "hello #1";
char *msg2 = "hello #2";
char *msg3 = "hello #3";

main()
{  char inbuf[MSGSIZE];

   int p[2], j, pid;

   /* open pipe */

   if(pipe(p) == -1)
   {    perror("pipe call error");
        exit(1);
   }
    pid = fork();
    printf("%s,%d, pid = %d\n", __func__, __LINE__, pid);
    char *cat_args[] = {"cat", "test", NULL};
    char *grep_args[] = {"/usr/sbin/dnsmasq", "--no-hosts",NULL};
   switch(pid){
   case -1: perror("error: fork call");
            exit(2);
   case 0:  /* if child then write down pipe */
         close(p[1]);  /* first close the write end of the pipe */
         if(dup2(p[0], 0 ) == -1 ) /* stdin == read end of the pipe */
         {
	     perror( "dup2 failed" );
	     _exit(1);
	 }
	  close(p[0]); /* close the fd of read end of the pipe */

	 if (execv("/usr/sbin/dnsmasq", grep_args) < 0) { /*dnsmasq can read STDIN_FILENO get father thread p[1] write information*/
            printf("%s,%d, errno = %s\n", __func__, __LINE__, strerror(errno));
            _exit(1);
        }
         break;
   default:   /* parent reads pipe */
         close(p[0]);  /* first close the read end of the pipe */
         if(dup2(p[1], 1 ) == -1 ) /* stdout == write end of the pipe */
         {
	     perror( "dup2 failed" );
	     _exit(1);
	 }
         printf("i am lcc");
         break;
   }

   exit(0);
}

