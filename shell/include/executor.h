#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"

void executeCommand(const Command &cmd);

pid_t executePipeline(const std::vector<Command>& commands, bool background);

#endif // EXECUTOR_H
