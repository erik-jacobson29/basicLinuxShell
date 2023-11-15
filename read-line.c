/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
int index;
char line_buffer[MAX_BUFFER_LINE];

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [200]; 
  // "ls -al | grep x", 
  // "ps -e",
  // "cat read-line-example.c",
  // "vi hello.c",
  // "make",
  // "ls -al | grep xxx | grep yyy"

//int history_length = sizeof(history)/sizeof(char *);

int history_length = 0; //initialize counter to 0

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing. s
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  index = line_length;
  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32) {
      // It is a printable character. 

      // Do echo
      write(1,&ch,1);
      //index++;
      // If max number of character reached return.
      if (line_length==MAX_BUFFER_LINE-2) break; 
      //but now we have to check so we don't overwrite a character
      //basically keep all the characters in front of the index the same
      //set the character at the index to the character typed
      if(index != line_length) {
        line_length++;
        for(int i = line_length; i > index; i--) {
            //shift all the characters up 1
            line_buffer[i] = line_buffer[i-1];
        }
        line_buffer[index] = ch; //writes the character at the index
        index++;
      
      for(int i = index; i < line_length; i++) {
        ch = line_buffer[i];
        write(1,&ch,1);
      }
      //now this is going to bring you all the way back to the end of the string
      for(int i = index; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);
      }
      }
      // add char to buffer.
      //if its at the end of the line
      else {
        index++;
        line_buffer[line_length]=ch;
        line_length++;
      }
    }
    else if (ch==10) {
      // <Enter> was typed. Return line

      // Print newline
      write(1,&ch,1);

      //now I want to add the line to the history array
      

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if (ch == 8) { //ctrl-h handling
      // <backspace> was typed. Remove previous character read.
      //first need to make sure we dont backspace into the prompt
      if(index >0) {
        if(index != line_length) {
          ch = 8;
          write(1,&ch,1);
          for(int i = index -1; i < line_length; i++) {
            //shift all the characters down 1
            line_buffer[i] = line_buffer[i+1];
            //line_length --;
        }
        for(int i = index-1; i < line_length; i++) {
          ch = line_buffer[i];
          write(1,&ch,1);
        }
        ch = ' ';
        write(1,&ch,1);
        index--;
        line_buffer[line_length] = 0;
        for (int i = 0; i < line_length-index; i++) {
          ch=8;
          write(1,&ch,1);
        }
        line_length--;
        
        }
      // Go back one character
      else {
        ch = 8;
        write(1,&ch,1);

        // Write a space to erase the last character read
        ch = ' ';
        write(1,&ch,1);

        // Go back one character
        ch = 8;
        write(1,&ch,1);

        // Remove one character from buffer
        line_length--;
        index--;
      }
      }
    }
    else if (ch == 4) { //delete and ctrl-dhandling
      // <backspace> was typed. Remove previous character read.
    
      // Go back one character
      // ch = 127;
      // write(1,&ch,1);
      if(index == line_length-1) {
      ///////THE BELOW CODE WORKS FOR DELETING THE LAST CHAR
      if(index >= 0) {
      // Write a space to erase the last character read
      ch = ' ';
      write(1,&ch,1);

      // Go back one character
      ch = 8;
      write(1,&ch,1);

      // Remove one character from buffer
      line_length--;
      //NOTE INDEX does not change
      }
      }
      
    
    else { //////////////DELETING NOT THE LAST CHARACTER
          if(index >=0) {
            // ch = 8;
            // write(1,&ch,1);
            for(int i = index; i < line_length; i++) {
              line_buffer[i] = line_buffer[i+1];
            } //shifts everything down one
            for(int i = index; i < line_length-1; i++) {
              ch = line_buffer[i];
              write(1,&ch,1);
            }
            ch = ' ';
            write(1,&ch,1);
            if(index >= line_length-1 ) {
            //index--;
            ch=8;
            write(1,&ch,1);
            }
            line_buffer[line_length] =0;
            for(int i = 0; i < line_length-index; i++) {
              ch = 8;
              write(1,&ch,1);
            }
            line_length--;

          }
    }
    }
    else if(ch ==1) { //this is control A
      while(index > 0) {
        ch = 8;
        write(1,&ch,1);
        index--;
      }
    }
    else if(ch == 5) { //this is control E handling.
      while(index < line_length) {
        ch = line_buffer[index];
        write(1,&ch,1);
        index++;
      }
    }
    
      
    
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);

      /////////LEFT ARROW/////////////////
      if (ch1 == 91 && ch2 == 68) { //this is handling of the left arrow
        if(index > 0) {
        ch = 8;
        write(1,&ch,1); //write an 8
        //decrement the index
        index --; //decrement the index
        }
  }
  ///////////RIGHT ARROW//////////////////
      if(ch1 == 91 && ch2 == 67) {
        if(index < line_length) {
        ch = line_buffer[index];
        write(1,&ch,1);
        index++;
        }
        
      }

      //////////////UP ARROW///////////////
      if (ch1==91 && ch2==65) {
	// Up arrow. Print next line in history.

	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
    //printf("here\n");
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}	
      
  

	// Copy line from history
  line_buffer[0] = 0; //clear the buffer
	strcpy(line_buffer, history[history_index]);
  //history_index --;
  
	line_length = strlen(line_buffer);
	//history_index=(history_index+1)%history_length;
  if(history_index >= 0) {
    if(history_index != 0) {
      history_index--;
    }
  }
  
  

	// echo line 
	write(1, line_buffer, line_length);
  index = line_length;
  
  }

    ///////////DOWN ARROW///////////////
  else if (ch1==91 && ch2==66) {
	// Down Arrow. Print next line in history.

	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
    //printf("here\n");
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}	
      
  

	// Copy line from history
	strcpy(line_buffer, history[history_index]);
	line_length = strlen(line_buffer);
  if(history_index <= history_length) {
    if(history_index != history_length -1) {
	    history_index++;
    }
  }
  

	// echo line 
	write(1, line_buffer, line_length);
  index = line_length;
      }
      
    }
  }

  


  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;

  history[history_length] = (char*)malloc(50); //malloc a pointer
  strcpy(history[history_length], line_buffer);
  history[history_length][strlen(line_buffer)-1] = 0;
  history_length++;
  history_index = history_length -1;

  return line_buffer;
}

