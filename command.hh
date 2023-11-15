#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"

// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  std::string * _lastArg;
 
  
  bool _background;
  //add counts of the number of outfiles
  int _outFileCount;
  //keeps track of if it is in append mode
  int _append;
  //keeps track of if the command is being run in the background
  int _lastcode;
  int _lastpid;
  


  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();
  std::string get_last_arg();

  static SimpleCommand *_currentSimpleCommand;
};

#endif
