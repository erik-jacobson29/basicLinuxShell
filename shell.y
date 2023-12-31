
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <cstring>
#include "command.hh"

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE GREATGREAT AMPERSAND GREATAMPERSAND PIPE LESS TWOGREAT GREATGREATAMPERSAND 

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void expandWildcardsIfNecessary(std::string * arg);
void yyerror(const char * s);
int yylex();

%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
  pipe_list io_modifier_list background NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;
pipe_list:
  pipe_list PIPE command_and_args 
  | command_and_args
;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    //Command::_currentSimpleCommand->insertArgument( $1 );
    //printf("here\n");
    Command::_currentSimpleCommand->expandWildcardsIfNecessary( $1->c_str() );
    
    
  }
  ;

command_word:
  WORD {
    //check to see if the word is exit 
    if(!strcmp($1->c_str(), "exit")){
      printf("\n Good Bye!! \n\n");
      exit(2);
    }
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;
background:
  AMPERSAND {
    //printf("   Yacc: insert background\"\n");
    Shell::_currentCommand._background = 1;

  }
    | 
  
  ;
io_modifier_list:
  io_modifier_list iomodifier_opt
  | 
  ;
iomodifier_opt:
  GREAT WORD {
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._outFileCount ++;
  }
  | GREATGREAT WORD {
 	  //printf("   Yacc: append output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._append = 1;
  }
  | GREATAMPERSAND WORD {
    //printf("   Yacc: redirect to output and error \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._errFile = $2;

  }
  | LESS WORD {
	  //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
    Shell::_currentCommand._inFile = $2;
  }
  | TWOGREAT WORD {
	  //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
    Shell::_currentCommand._errFile = $2;
  }
  | GREATGREATAMPERSAND WORD {
    //printf("   Yacc: append to output and error \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._errFile = $2;
  }
  
  ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
