#ifndef FALLOUT_SFALL_HOOKS_H_
#define FALLOUT_SFALL_HOOKS_H_

#include "scripts.h"


namespace fallout {

    
enum HookType {
    HOOK_TOHIT = 0,
    HOOK_AFTERHITROLL = 1,
    HOOK_CALCAPCOST = 2,
    HOOK_DEATHANIM1 = 3,
    HOOK_DEATHANIM2 = 4,
    HOOK_COMBATDAMAGE = 5,
    HOOK_ONDEATH = 6,
    HOOK_FINDTARGET = 7,
    HOOK_USEOBJON = 8,
    HOOK_REMOVEINVENOBJ = 9,
    HOOK_BARTERPRICE = 10,
    HOOK_MOVECOST = 11,
    HOOK_HEXMOVEBLOCKING = 12,
    HOOK_HEXAIBLOCKING = 13,
    HOOK_HEXSHOOTBLOCKING = 14,
    HOOK_HEXSIGHTBLOCKING = 15,
    HOOK_ITEMDAMAGE = 16,
    HOOK_AMMOCOST = 17,
    HOOK_USEOBJ = 18,
    HOOK_KEYPRESS = 19,
    HOOK_MOUSECLICK = 20,
    HOOK_USESKILL = 21,
    HOOK_STEAL = 22,
    HOOK_WITHINPERCEPTION = 23,
    HOOK_INVENTORYMOVE = 24,
    HOOK_INVENWIELD = 25,
    HOOK_ADJUSTFID = 26,
    HOOK_COMBATTURN = 27,
    HOOK_CARTRAVEL = 28,
    HOOK_SETGLOBALVAR = 29,
    HOOK_RESTTIMER = 30,
    HOOK_GAMEMODECHANGE = 31,
    HOOK_USEANIMOBJ = 32,
    HOOK_EXPLOSIVETIMER = 33,
    HOOK_DESCRIPTIONOBJ = 34,
    HOOK_USESKILLON = 35,
    HOOK_ONEXPLOSION = 36,
    HOOK_SUBCOMBATDAMAGE = 37,
    HOOK_SETLIGHTING = 38,
    HOOK_SNEAK = 39,
    HOOK_STDPROCEDURE = 40,
    HOOK_STDPROCEDURE_END = 41,
    HOOK_TARGETOBJECT = 42,
    HOOK_ENCOUNTER = 43,
    HOOK_ADJUSTPOISON = 44,
    HOOK_ADJUSTRADS = 45,
    HOOK_ROLLCHECK = 46,
    HOOK_BESTWEAPON = 47,
    HOOK_CANUSEWEAPON = 48,
    HOOK_COUNT
};

void registerHook(Program* program, int id, int proc, bool spec);
void RunHook(int id);
void RunKeyPressHook(int pressed, int v1, int v2);
void RunCombatTurnHook(int critter, int dudeBegin);


void get_sfall_arg(Program* program);
void op_get_sfall_args(Program* program);
void set_sfall_return(Program* program);
void op_set_sfall_arg(Program* program);

} // namespace fallout

#endif /* FALLOUT_SFALL_HOOKS_H_ */
