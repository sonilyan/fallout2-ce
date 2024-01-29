#include "sfall_hooks.h"

namespace fallout {


constexpr int maxArgs = 16; // Maximum number of hook arguments
constexpr int maxRets = 8; // Maximum number of return values
constexpr int maxDepth = 8;

enum class DataType : unsigned long {
    NONE = 0,
    INT = 1,
    FLOAT = 2,
    STR = 3,
};

bool allowNonIntReturn;

DataType retTypes[maxRets]; // current hook return value types
int args[maxArgs]; // current hook arguments
int rets[maxRets]; // current hook return values
int argCount;
int cArg; // how many arguments were taken by current hook script
int cRet; // how many return values were set by current hook script
int cRetTmp; // how many return values were set by specific hook script (when using register_hook)


struct HookScript {
    Program* program;
    int proc;
    bool isGlobalScript;
};

std::vector<HookScript> hooks[HOOK_COUNT];

void registerHook(Program* program, int id, int proc, bool spec)
{
    if (id >= HOOK_COUNT)
        return;
    for (std::vector<HookScript>::iterator it = hooks[id].begin(); it != hooks[id].end(); ++it) {
        if (it->program == program) {
            if (proc == 0) hooks[id].erase(it); // unregister
            return;
        }
    }
    if (proc == 0)
        return;

    HookScript hook;
    hook.program = program;
    hook.proc = proc;
    hook.isGlobalScript = true;

    auto c_it = hooks[id].cend();
    if (spec) {
        c_it = hooks[id].cbegin();
        //hooksInfo[id].hsPosition++;
    }
    hooks[id].insert(c_it, hook);
}

void RunHook(int id)
{
    cArg = 0;
    cRet = 0;
    cRetTmp = 0;

    if (id >= HOOK_COUNT || hooks[id].empty())
        return;

    for (std::vector<HookScript>::iterator it = hooks[id].begin(); it != hooks[id].end(); ++it) {
        if (it->proc == -1)
            continue;
        _executeProcedure(it->program, it->proc);
    }
}


void RunCombatTurnHook(int critter, int dudeBegin)
{
    argCount = 3;
    args[0] = 1;
    args[1] = critter;
    args[2] = dudeBegin;

    RunHook(HOOK_KEYPRESS);
    /*
    if (cRet != 0) {
        long retKey = rets[0];
        if (retKey > 0 && retKey < 264) *dxKey = retKey;
    }
    */
}


void RunMouseClickHook(int button, int pressed)
{
    argCount = 2;
    args[0] = pressed;
    args[1] = button;
    RunHook(HOOK_MOUSECLICK);
}

void RunKeyPressHook(int pressed,int v1,int v2)
{
    argCount = 3;
    args[0] = pressed;
    args[1] = v1;
    args[2] = v2;
    RunHook(HOOK_KEYPRESS);
    /*
    if (cRet != 0) {
        long retKey = rets[0];
        if (retKey > 0 && retKey < 264) *dxKey = retKey;
    }
    */
}

 void get_sfall_arg(Program* program)
{
     programStackPushInteger(program, (cArg == argCount) ? 0 : args[cArg++]);
 }

 void op_get_sfall_args(Program* program)
{
    programStackPushInteger(program, 0);
}

 void set_sfall_return(Program* program)
{
    programStackPopValue(program);
}

 void op_set_sfall_arg(Program* program)
{
}

} // namespace fallout

