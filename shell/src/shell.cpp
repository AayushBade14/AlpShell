#include "shell.h"

// normal loop without arguments
void AlpShell::alp_loop(){

}

// loop with arguments
void AlpShell::alp_loop(char **flags){

}

// method for prompt
void AlpShell::alp_prompt(){

}

// method to run shell with flags
void AlpShell::run(char **flags){
  AlpShell::alp_loop(flags);
}

// method to run shell without flags
void AlpShell::run(){
  AlpShell::alp_loop();
}

