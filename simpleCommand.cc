#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <regex.h>
#include "shell.hh"
#include <pwd.h>
#include <cassert>
#include <dirent.h>
#include <bits/stdc++.h>
//#include "simpleCommand.hh"
#define MAXFILENAME 1024

//initialize global variables
char ** array;
int maxEntries;
int nEntries;

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}



void SimpleCommand::insertArgument( std::string * argument ) {
  //now we have to parse the command to see if its Environment Variable expansion
  char * arg = (char *)argument->c_str(); //save the argument
  std::string output = ""; //make the output string

//implement tilde expansion
int start = 0;
if(arg[start] == '~') { //sees if the first character is a tilde
  if(strlen(arg) == 1) {
    output += getenv("HOME");
  }
  else {
    output += "/homes/";
  }
  arg++;
}
  //check to see if the first character is a '$' for variable expansion
  while(*arg != '\0') {
  //for(int i = 0; i < (int)strlen(arg); i++) {
    if (*arg == '$' && *(arg + 1) == '{') {
    
    //then we have to check for variable expansion.
    //get rid of the first two and last characters
    char * inside = (char*) malloc (300); //malloc 100 bytes
    char* location = inside; // be able to keep track of where you are in the string
    arg += 2; //move the pointer so we can begin writing to the temp string
    while(*arg != '}') {
      
      //loop through and add the characters to inside
      *location = *arg;
      //increment both pointers
      location ++;
      arg ++;
    }
    arg++;
    
    //printf("Testing: %s\n", inside); //->>this works
    *location = '\0';
    //now once we have the argument, we need to get that environment variable
    
    
    if(strcmp(inside, "$") == 0) { //checks the first special case
      std::string env1 = std::to_string((int)getpid()); //returns the pid
      output+=env1;
    }
    else if(strcmp(inside, "?") == 0) { //checks for return code
      std::string env2 = std::string(std::to_string(Shell::_currentCommand._lastcode));
      output += env2;
    }
    else if(strcmp(inside, "!") == 0) { //returns the pid of the last process
      std::string env3 = std::to_string(Shell::_currentCommand._lastpid);
      output += env3;
    }
    else if(strcmp(inside, "SHELL") == 0) { // handles the shell case and gets the path
      //get the real path
      char *path = (char*)malloc(100);
      realpath("../shell" , path);
      std::string env4 = std::string(path);
      output += env4;
    } 
    else if(strcmp(inside, "_") == 0) {
      std::string env5 = std::string(Shell::_currentCommand.get_last_arg().c_str());
      output+= env5;
    }
    else {
      char * env = getenv(inside);
      //printf("Testing %s\n", env); //->> seems like it works
      if (env != NULL) {
        output+=env;
    }
    }

  }
  else{ //otherwise just add the char to the output string
    output += *arg;
    arg++;
  }
}

std::string * a = new std::string(output);


  // simply add the argument to the vector
  _arguments.push_back(a);
}

//now I have to implement wildcard 
void SimpleCommand::expandWildcards(char * prefix, char * suffix) {
  if (suffix[0] == 0) { //This is the base case
    if(nEntries == maxEntries) { //expand the maxEntries and the array
      maxEntries *= 2;
      array = (char**)realloc(array, maxEntries*sizeof(char*)); //realloc the array
    }
    prefix +=2;
    suffix++;
    array[nEntries] = strdup(prefix);
    nEntries ++;
    return;
  }
  char * temp = strchr(suffix,'/'); //points to the first '/' in suffix
  char component[MAXFILENAME];
  //now we want to copy up to the first '/'
  if(temp != NULL) {
      strncpy(component,suffix, temp-suffix); //copies that many characters into the string
      suffix = temp + 1; //makes the new suffix thats everything after the '/' character
  }
  else {
    //temp is null so there is no slash
    strcpy(component,suffix); //just copy the whole suffix into component
    suffix += strlen(suffix);
  }

  //now we want to expand the suffix
  char pnew[MAXFILENAME];
  if(strchr(component, '*') == NULL && strchr(component, '?') == NULL) {
    //then the suffix has another wildcard so we need to make a recursive call
    sprintf(pnew, "%s/%s", prefix, component); //write to pnew
    expandWildcards(pnew, suffix); //make a recursive call to expand wildcard
    return;
  }

  //copied code from expandWildcardsIfNecessary
  char * reg =(char*) malloc (2 * strlen(component) + 10); //malloc a bunch of bytes for the regular 
    char * a = component;
    char * r = reg; //fabove lines of code from the slides
    *r = '^';
    r++; //matches the beginning of the line
    while(*a) {
      if (*a == '*') {
        *r = '.';
        r++;
        *r = '*';
        r++;
      }
      else if (*a == '?') { //expanding all of the variables
        *r = '.';
        r++;
      }
      else if (*a == '.') {
        *r = '\\';
        r++;
        *r = '.';
        r++;
      }
      else {
        *r = *a;
        r++;
      }
      a++; //increment a to move through the string
    }
    *r = '$';
    r++;
    *r = 0; //match the end of the line and add null char
    //END OF STEP 1 code from the slides

    //STEP 2: compile the regular expression
    regex_t regex;
    int expbuf = regcomp(&regex, reg, REG_EXTENDED|REG_NOSUB); //matches the regular expression
        
    if(expbuf) { // if it matches
      perror("compile");
      return;
    }

    //make a char* for the directory name
    char * dir;
    if (prefix == NULL) {
      dir = "."; //list the current directory
    }
    else {
      dir = prefix;
    }

    DIR * d = opendir(dir);
    if(d == NULL) {
      return;
    } 
    //same as expand as necessary
    regmatch_t pmatch[1];
    struct dirent * ent;

    while((ent = readdir(d)) != NULL) {
      if (!regexec(&regex, ent->d_name, 1, pmatch, 0)) {
        if(ent->d_name[0] == '.') {
          if(component[0] == '.') {
            sprintf(pnew, "%s/%s", prefix, ent->d_name);
            expandWildcards(pnew,suffix);
          }
        }
        else {
          sprintf(pnew, "%s/%s", prefix, ent->d_name);
          expandWildcards(pnew,suffix);
        }
      }
    }
    regfree(&regex);
    free(reg);
    closedir(d);
        
}
//now we have to implement wildcard expansion
void SimpleCommand::expandWildcardsIfNecessary(const char* arg) {
    //first we want to simply return arg if there is no '*' or "?"
    if(strchr(arg,'*') == NULL && (strchr(arg, '?') == NULL || strstr(arg, "{?}") != NULL)) { //has no wildcards
      std::string * argument = new std::string(arg); // make a new pointer
      Command::_currentSimpleCommand->insertArgument(argument); // add the argument like normal
      free(argument);
      return;
    }
    
    //so if it does contain a wildcard
    //step 1: conver the wildcard to a regular expression
    char * reg =(char*) malloc (2 * strlen(arg) + 10); //malloc a bunch of bytes for the regular 
    char * a = (char*)arg;
    char * r = reg; //fabove lines of code from the slides
    *r = '^';
    r++; //matches the beginning of the line
    while(*a) {
      if (*a == '*') {
        *r = '.';
        r++;
        *r = '*';
        r++;
      }
      else if (*a == '?') { //expanding all of the variables
        *r = '.';
        r++;
      }
      else if (*a == '.') {
        *r = '\\';
        r++;
        *r = '.';
        r++;
      }
      else {
        *r = *a;
        r++;
      }
      a++; //increment a to move through the string
    }
    *r = '$';
    r++;
    *r = 0; //match the end of the line and add null char
    //END OF STEP 1 code from the slides

    //STEP 2: compile the regular expression
    regex_t regex;
    maxEntries = 20; //from the slides 
    nEntries = 0;
    array = (char**) malloc(maxEntries * sizeof(char*)); //malloc bytes for the array
    DIR * dir = opendir(".");
    if(strchr(arg, '/') == NULL) {
        int expbuf = regcomp(&regex, reg, REG_EXTENDED|REG_NOSUB); //matches the regular expression
        if(expbuf) { // if it matches
          perror("compile");
          return;
        }

        //STEP 3 is to list the directory and add as arguments the entries
        if(dir == NULL) { //bad directory than simply return
          perror("opendir");
          return;
        }
        //more code from the slides
        struct dirent * ent;
        regmatch_t pmatch[1];
        //printf("%s\n", reg);

        while((ent =readdir(dir)) != NULL) {
          //check if the name matches
          if(!regexec(&regex, ent->d_name, 1, pmatch, 0)) {
            if (nEntries == maxEntries) {
              //double the maxEntries
              maxEntries *= 2;
              //also double the size of the array
              array = (char**)realloc(array,maxEntries*sizeof(char*));
              assert(array != NULL);

            }
            if (ent->d_name[0] == '.') {
              if(arg[0] == '.') { //check to see if there is a period
                array[nEntries] = strdup(ent->d_name);
                nEntries++;
              }
            }
            else {
              array[nEntries] = strdup(ent->d_name);
              nEntries++;
            }
          }
        }
        //now we want to free everything
        regfree(&regex);
        free(reg);
        closedir(dir);
    }
    else { //call expand wildcards
          
          expandWildcards((char*)"/", (char*)arg);
        }

    for(int i = 0; i < nEntries; i++) {
      int min = i;
      for (int j = i+1; j < nEntries; j++) {
        if(strcmp(array[j], array[min]) < 0) {
          min = j;
        }
      }
      char * temp = array[i];
      array[i] = array[min];
      array[min] = temp;
    }

    if(array[0] == NULL) { //just add the initial argument
      Command::_currentSimpleCommand->insertArgument(new std::string(arg));
    }
    else {
      //add all the arguments
      for (int i = 0; i < nEntries; i++) {
        Command::_currentSimpleCommand->insertArgument(new std::string(array[i]));
        free(array[i]);
      }
    }

    free(array);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
