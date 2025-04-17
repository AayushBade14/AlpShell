#include "history.h"
#include "prompt.h"
#include <iostream>
#include <vector>
#include <iomanip>

static std::vector<std::string> history;

namespace History {

    void addHistory(const std::string &command) {
        history.push_back(command);
    }

    void printHistory()
    {
        int index = 1;
        for (const auto &cmd : history)
        {
            std::cout << COLOR_BLUE << std::setw(4) << index++ << COLOR_RESET
                      << " " << COLOR_CYAN << cmd << COLOR_RESET << std::endl;
        }
    }

    const std::vector<std::string>& getHistory() {
        return history;
    }
}
