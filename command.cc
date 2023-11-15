/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "command.hh"
#include "shell.hh"
//adding these headers
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

//add a global variable for printenv
extern char ** environ;
int setenvb = 0;
int cdb = 0; //both two variables to prevent segfaults


Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
   // _lastArg = NULL;
    _background = false;
    _outFileCount = 0;
    _append  = 0; //append is set to false
    //add two more for the special variable expansions
    _lastcode = 0;
    _lastpid = 0;
    
}

std::string lastArg = "";
std::string Command::get_last_arg() {
   return lastArg;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();
   
    if ( _outFile ) {
        delete _outFile;
    }

    else if ( _errFile ) {
        delete _errFile;
    }
    _outFile = NULL;
    _errFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;
    _background = false;

    //reset the outcount to 0
    _outFileCount = 0;
}


void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}



void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }
    

    //CHECK TO SEE IF THERE ARE MULTIPLE REDIRECTS
    if(_outFileCount > 1) {
        printf("Ambiguous output redirect.\n");
	clear();
	Shell::prompt();

        return;
    }

    // Print contents of Command data structure
    //print();

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    int ret;
    int tmpin = dup(0);
    int tmpout = dup(1);
    int tmperr = dup(2);
    //handle input redirection
    int fdin;
    if(_inFile) { //then we want to set fdin to infile
        const char * in = _inFile->c_str();
        fdin = open(in,O_RDONLY);
    }
    else { //just set fdin to standard in
        fdin = dup(tmpin);

    }
    int fdout;
    int fderr;
    if(_errFile) {
        
        if(_append) {
            fderr = open(_errFile->c_str(), O_APPEND | O_WRONLY | O_CREAT, 0600);
        }
        else {
            fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        }
    }
    else {
        fderr = dup(tmperr);
    }
    dup2(fderr,2);
    close(fderr);
    for (int i = 0; i < (int)_simpleCommands.size(); i++) {
        //first thing i want to do is populate _lastArg
        
        //lets set up i/o redirection for this part
        dup2(fdin, 0);
        close(fdin);
	
        if(i == (int)_simpleCommands.size() -1) { //check to see if its the last command
        
            
            if(_outFile) { //check to see if there is an outfile redirection
		    //check to see if it is append or not
		            const char * out = _outFile->c_str();
		            if(_append == 1) {
			            fdout = open(out, O_CREAT | O_WRONLY | O_APPEND, 0600);
		            }
		            else {
                        fdout=open(out, O_CREAT|O_RDWR|O_TRUNC, 0600);
		            }
	
            }
            else{
                fdout = dup(tmpout);
            }
            
        }
         
        else {
            //not the last command
            int fdpipe[2];
            pipe(fdpipe);
            fdout = fdpipe[1];
            fdin = fdpipe[0];
        }
        dup2(fdout,1);
        close(fdout);
        // dup2(fderr,2);
        // close(fderr);
        //check for setenv
        if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv")) { //the command is setenv
            setenvb = 1;
            setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1);
            clear();
            Shell::prompt();
        }

        //now check for unsetenv
        else if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv")) { //the command is unsetenv
            unsetenv(_simpleCommands[i]->_arguments[1]->c_str());
            clear();
            Shell::prompt();
        }
        //now check for cd
        else if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd")) { // the command is cd
            //check to see the number of arguments
            cdb = 1;
            if(_simpleCommands[i]->_arguments[1] == NULL) {
                //there is only one argument
                chdir(getenv("HOME"));
            }
            else {
                //there is a specified directory
                if(chdir(_simpleCommands[i]->_arguments[1]->c_str()) != 0) {
                    //tries to change directory but there is an error
                    fprintf(stderr, "cd: can't cd to %s\n", _simpleCommands[i]->_arguments[1]->c_str());
                    //above line just prints the error
                }
            }
        
            clear();
            Shell::prompt();
            
        }
        
        //forks a new process
        ret = fork();
        if(ret==0) {
            
            const char * command = _simpleCommands[i]->_arguments[0]->c_str();
            int numargs = _simpleCommands[i]->_arguments.size();
            char ** args = new char* [numargs + 1];
	        args[numargs] = NULL;
            for(int j =0; j < numargs; j++){
                const char * temp =  _simpleCommands[i]->_arguments[j]->c_str();
                args[j] = (char*)temp;
            }

            //check to see if the command is printenv
            if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv")) {
                char ** p = environ;
                while(*p != NULL) { //loop through all the values in the double pointer and print them
                    printf("%s\n", *p);
                    p+=1;
                }
                exit(0);
                
            }
            //now execute the command
	    
            execvp(command, args);
            perror("execvp");
            _exit(1);
    }
        else if (ret < 0) {
            perror("fork");
            exit(2);
        }
        if(! setenvb && !cdb) {
            lastArg = _simpleCommands[i]->_arguments[_simpleCommands[i]->_arguments.size()-1]->c_str();
        }
        
    }


    //restore in/out defaults
    dup2(tmpin, 0);
    dup2(tmpout, 1);
    dup2(tmperr,2);
    close(tmpin);
    close(tmpout);
    close(tmperr);
    close(fdin);
    

    //add the part that waits for the background
    if(_background == 0) {
        int temp = 0;
	    waitpid(ret, &temp, 0); //this waits for the last process to finish
        //add in a line that stores the exit code in the global variable
        _lastcode = WEXITSTATUS(temp);
    }
    else {
        //update the last_pid class variabel
        _lastpid = ret;
    }


    // Clear to prepare for next command
    clear();

    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
