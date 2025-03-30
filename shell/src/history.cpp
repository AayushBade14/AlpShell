#include "history.h"
#include <iostream>
#include <vector>

static std::vector<std::string> history;

namespace History {

    void addHistory(const std::string &command) {
        history.push_back(command);
    }

    void printHistory() {
        int index = 1;
        for (const auto &cmd : history) {
            std::cout << index++ << " " << cmd << std::endl;
        }
    }

    const std::vector<std::string>& getHistory() {
        return history;
    }
}
