#include "sfall_hooks.h"
#include "debug.h"
#include "game_dialog.h"
#include "item.h"
#include "inventory.h"
#include "sfall_arrays.h"
#include "interpreter.h"

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
ProgramValue args[maxArgs]; // current hook arguments
int rets[maxRets]; // current hook return values
int argCount;
int cArg; // how many arguments were taken by current hook script
int cRet; // how many return values were set by current hook script
int cRetTmp; // how many return values were set by specific hook script (when using register_hook)


ProgramValue GetHSArgAt(int id)
{
    return args[id];
}

struct HookScript {
    Program* program;
    int proc;
    bool isGlobalScript;
};

std::vector<HookScript> hooks[HOOK_COUNT];

void sfall_hooks_reset()
{
    for (int i = 0; i < HOOK_COUNT; i++) {
        hooks[i].clear();
    }
}

void sfall_hooks_clear(Program *program)
{
    if (program->hook == true) {
        for (int id = 0; id < HOOK_COUNT; id++) {
            for (std::vector<HookScript>::iterator it = hooks[id].begin(); it != hooks[id].end();) {
                if (it->program == program) {
                    it = hooks[id].erase(it);
                    debugPrint("sfall_hooks_clear %s proc=%d", program->name, id);
                } else {
                    it++;
                }
            }
        }

        program->hook = false;
    }
}
    
void registerHook(Program* program, int id, int proc, bool spec)
{
    if (id >= HOOK_COUNT)
        return;
    for (std::vector<HookScript>::iterator it = hooks[id].begin(); it != hooks[id].end(); ++it) {
        if (it->program == program) {
            if (proc == 0) {
                program->hook = false;
                hooks[id].erase(it); // unregister
            }
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
    program->hook = true;
    debugPrint("registerHook %x %s value=%d proc=%d spec=%d", program, program->name, id, proc, spec ? 1 : 0);
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

void RunBarterPriceHook(Object* source, Object* target, bool offers_button_pressed)
{
    bool barterIsParty = gGameDialogSpeakerIsPartyMember;
    long computeCost = _barter_compute_value2(source, target);

    argCount = 10;

    args[0] = ProgramValue(source);
    args[1] = ProgramValue(target);
    args[2] = ProgramValue (!barterIsParty ? computeCost : 0);

    Object* bTable = _btable;
    args[3] = ProgramValue(bTable);
    args[4] = ProgramValue(itemGetTotalCaps(bTable));
    args[5] = ProgramValue(objectGetCost(bTable));

    Object* pTable = _ptable;
    args[6] = ProgramValue(pTable);

    long pcCost = 0;
    if (barterIsParty) {
        args[7] = ProgramValue(pcCost);
        pcCost = objectGetInventoryWeight(pTable);
    } else {
        args[7] = ProgramValue(pcCost = objectGetCost(pTable));
    }

    args[8] = ProgramValue(offers_button_pressed ? 1 : 0); // offers button pressed
    args[9] = ProgramValue((int) barterIsParty);

    RunHook(HOOK_BARTERPRICE);
}

void RunMoveItemHook(Object* from, Object* to, Object* item, int quantity,int wtf)
{
    argCount = 5;
    args[0] = ProgramValue(from);
    args[1] = ProgramValue(item);
    args[2] = ProgramValue(quantity);
    args[3] = ProgramValue(wtf);
    args[4] = ProgramValue(to);

    RunHook(HOOK_REMOVEINVENOBJ);
}


void RunCombatTurnHook(Object* critter, int dudeBegin)
{
    argCount = 3;
    args[0] = ProgramValue(1);
    args[1] = ProgramValue(critter);
    args[2] = ProgramValue(dudeBegin);

    RunHook(HOOK_COMBATTURN);
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
    args[0] = ProgramValue(pressed);
    args[1] = ProgramValue(button);
    RunHook(HOOK_MOUSECLICK);
}

void RunKeyPressHook(int pressed,int v1,int v2)
{
    argCount = 3;
    args[0] = ProgramValue(pressed);
    args[1] = ProgramValue(v1);
    args[2] = ProgramValue(v2);
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
    ProgramValue value = ProgramValue(0);
    if (cArg == argCount)
        programStackPushValue(program,value);
    else
        programStackPushValue(program, args[cArg++]);
}

 void op_get_sfall_args(Program* program)
 {
     ArrayId result = CreateTempArray(argCount, 0); // associative
     
     for (int i = 0; i < argCount; i++) {
         SetArray(result, ProgramValue(i), args[i], false, program);
     }

     programStackPushInteger(program, result);
 }

 void set_sfall_return(Program* program)
{
    programStackPopValue(program);
}

 void op_set_sfall_arg(Program* program)
{
    int id = programStackPopInteger(program);
    if (id < argCount)
        args[id] = programStackPopValue(program);
}

} // namespace fallout

