
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <iostream>
#include <string>
#include "y.tab.hh"
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "y.tab.hh"
#include <string.h>
#include "shell.hh"
// START ADDED CODE FROM THE SLIDES //
extern "C" char * read_line();

int mygetc(FILE * f) {
  static char * p;
  char ch;
  if(!isatty(0)) {
    return getc(f);
  }
  if(p==NULL || *p == 0) {
    char * s = read_line();
    p = s;
  }
  ch = *p;
  p++;
  return ch;
}
#undef getc
#define getc(f) mygetc(f)

//END of Added Code

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}


">>" {
	return GREATGREAT;
}

"|" {
	return PIPE;
}

"&" {
	return AMPERSAND;
}

"<" {
	return LESS;
}

"2>" {
	return TWOGREAT;
}

">&" {
	return GREATAMPERSAND;
}

">>&" {
	return GREATGREATAMPERSAND;
}
"source "[^\t\n][^\t\n]* {
  //this is the code to handle source.  //match the pattern and perform the steps
  std::string t = std::string(yytext); //get the string 
  t = t.substr(t.find(' ') + 1); //gets the name of the file
  FILE * f = fopen(t.c_str(), "r"); //open the file
  yypush_buffer_state(yy_create_buffer(f,YY_BUF_SIZE));
  yyparse();
  yypop_buffer_state();
  fclose(f);

}

\$\([^\t\n]*\) {  //this is all for the subshell
  //this is the regular expression for a dollar sign which is a call to the subshell
  //parent creates two pipes
  int pin[2], pout[2];
  pipe(pin); pipe(pout);
  //quickly remove the $ ()
  yylval.cpp_string = new std::string(yytext);
  yylval.cpp_string->erase(0,2); //removes the first two characters
  yylval.cpp_string->erase(std::prev(yylval.cpp_string->end())); //should remove the last character
  std::string exit = "\nexit\n";
  yylval.cpp_string->append(exit); //adds newline and a call to exit
  //printf("%s", output.c_str()); 
  //printf("%d\n", (int)yy.size()); 

  //write this to the pipe
  write(pin[1], yylval.cpp_string->c_str(), yylval.cpp_string->size()); //write the string to the pipe
  
  //close pin pipe that is not being used anymore
  close(pin[1]);

  

  //parent creates child process
  int ret;
  ret = fork();

  if (ret == 0) { //this is the child 
    dup2(pin[0],0); //redirects standard in
    close(pin[0]); //closes the pipe
    dup2(pout[1],1); //redirects standard out
    close(pout[1]); //closes the pipe
    execlp("/proc/self/exe","/proc/self/exe", NULL); //call own shell as a child process 
    
  }

  else if (ret < 0){
    perror("fork");
    _exit(1);
  }
  
  //wait for the child to finish then read in character by character build it into a string
  char * b = new char[4096];  //create an arbitrarily large buffer
  read(pout[0], b, 4092);
  close(pout[0]);
  close(pin[0]);

  //now we want to 
  std::string buffer = std::string((char *) b);
  buffer.pop_back();

  while(buffer.size() > 0) {
    char c = buffer.back();
    if (c == '\n') {
      c = ' ';
    }
    myunputc(c);
    buffer.pop_back();
  }

}

\"[^\n\"]*\" { 
//this is the regular expression for quotes
// do some operation to the yytext and yylval.string_val to remove the quotes 
yylval.cpp_string = new std::string(yytext);
yylval.cpp_string->erase(0,1); //removes the first character in the string
yylval.cpp_string->erase(std::prev(yylval.cpp_string->end())); //removes the last character in the string
return WORD; 
} 
[^ \t\n]*\\[^\t\n]* {
  //this is for escape
  //so this in theory means 0 or more non whitespaces followed by a slash followed by 0 or more 
  char * s = yytext; //turn it into a char * so i can loop through it
  std::string output; //create a string to append characters too two output
  int index = 0;
  for (int i = 0; i < strlen(s); i++) {
    //printf("%c\n", s[i]);
    if(s[i] != 92) { //the character is not a slash
      output += s[i];
    }
    else { //the character is a slash
      if (i + 1 != strlen(s)) {
        output += s[i + 1];
        i++;
      }
    }
  }
  yylval.cpp_string = new std::string(output);  //set yylval = output string
  return WORD;
}

[^ <>&|\t\n][^ <>&|\t\n]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  
  return WORD;
}












