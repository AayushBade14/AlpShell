/*
  ___   _     ______      ______  ___  ______  _____ ___________ 
 / _ \ | |    | ___ \     | ___ \/ _ \ | ___ \/  ___|  ___| ___ \
/ /_\ \| |    | |_/ /_____| |_/ / /_\ \| |_/ /\ `--.| |__ | |_/ /
|  _  || |    |  __/______|  __/|  _  ||    /  `--. \  __||    / 
| | | || |____| |         | |   | | | || |\ \ /\__/ / |___| |\ \ 
\_| |_/\_____/\_|         \_|   \_| |_/\_| \_|\____/\____/\_| \_|                                                                 
*/

#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include<iostream>
#include<fstream>
#include<sstream>

namespace AlpParser{
  char **alp_parse(char *ip);
};
