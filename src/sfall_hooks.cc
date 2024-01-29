#include "sfall_hooks.h"

namespace fallout {

/*
    	if (id >= numHooks) return;
	for (std::vector<HookScript>::iterator it = hooks[id].begin(); it != hooks[id].end(); ++it) {
		if (it->prog.ptr == script) {
			if (procNum == 0) hooks[id].erase(it); // unregister
			return;
		}
	}
	if (procNum == 0) return; // prevent registration to first location in procedure when reusing "unregister" method

	ScriptProgram *prog = ScriptExtender::GetGlobalScriptProgram(script);
	if (prog) {
		dlog_f("Script: %s registered as hook ID %d\n", DL_HOOK, script->fileName, id);
		HookScript hook;
		hook.prog = *prog;
		hook.callback = procNum;
		hook.isGlobalScript = true;

		auto c_it = hooks[id].cend();
		if (specReg) {
			c_it = hooks[id].cbegin();
			hooksInfo[id].hsPosition++;
		}
		hooks[id].insert(c_it, hook);

		HookScripts::InjectingHook(id); // inject hook to engine code
	}
    */

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
    if (id >= HOOK_COUNT)
        return;
    for (std::vector<HookScript>::iterator it = hooks[id].begin(); it != hooks[id].end(); ++it) {
        _executeProcedure(it->program, it->proc);
    }
}
} // namespace fallout
