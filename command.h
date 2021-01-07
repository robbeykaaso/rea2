#ifndef REAL_FRAMEWORK_COMMAND_H_
#define REAL_FRAMEWORK_COMMAND_H_

#include <functional>
#include "util.h"

namespace rea {

class DSTDLL ICommand{
public:
    ICommand(){}
    ICommand(std::function<void(void)> aRedo, std::function<void(void)> aUndo) : undo(aUndo), redo(aRedo){}
    std::function<void(void)> undo;
    std::function<void(void)> redo;
};

}

#endif
