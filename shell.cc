#include <cstdio>
#include <unistd.h>
#include "shell.hh"
#include <cstring>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int yyparse(void);

void Shell::prompt() {
  if( isatty(0)) { //check to see where the input is coming from
    printf("myshell>");
    fflush(stdout);
  }
  
}
//from ctrl-c.cc --- signal handling
extern "C" void disp( int sig )
{
	//fprintf("\n"); //print a new line for Ctrl + c
  Shell::_currentCommand.clear();
  fprintf(stderr, "\n");
  Shell::prompt();
  
  
}

//zombie
extern "C" void zomb( int sig )
{
	//fprintf("\n"); //print a new line for Ctrl + c
  Shell::_currentCommand.clear();
  while(waitpid(-1,NULL,WNOHANG) > 0);
  
  
}



int main() {
  Shell::prompt();

  //exit handling
  // char s[ 20];
  // fgets(s, 20, stdin);
  // if ( !strcmp( s, "exit\n" ) ) {
	// 		printf( "\nGood Bye!!\n\n");
	// 		exit( 1 );
	// 	}
  //ctrl C handling

  struct sigaction sa;
  sa.sa_handler = disp;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sa, NULL)){
      perror("sigaction");
      exit(2);
    }
  //end of CTRL C handling

  //zombie process handling
  struct sigaction saz;
  saz.sa_handler = zomb;
  sigemptyset(&saz.sa_mask);
  saz.sa_flags = SA_RESTART;

  if(sigaction(SIGCHLD, &saz, NULL)){
      perror("sigaction");
      exit(2);
    }
  //end of zombie process handling
  
  
  yyparse();
  
  
}

Command Shell::_currentCommand;
