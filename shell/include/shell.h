/* 
  ___   _     ______       _____ _   _  _____ _      _     
 / _ \ | |    | ___ \     /  ___| | | ||  ___| |    | |    
/ /_\ \| |    | |_/ /_____\ `--.| |_| || |__ | |    | |    
|  _  || |    |  __/______|`--. \  _  ||  __|| |    | |    
| | | || |____| |         /\__/ / | | || |___| |____| |____
\_| |_/\_____/\_|         \____/\_| |_/\____/\_____/\_____/                      
*/

#pragma once

#include "parser.h"
#include "util.h"

namespace AlpShell{
  
  
  void run(char **flags);
  void run();
  void alp_loop(char **flags);
  void alp_loop();
  void alp_prompt();

};
