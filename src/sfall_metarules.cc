#include "sfall_metarules.h"

#include <string.h>

#include "combat.h"
#include "debug.h"
#include "game.h"
#include "game_dialog.h"
#include "game_mouse.h"
#include "interface.h"
#include "inventory.h"
#include "object.h"
#include "sfall_ini.h"
#include "text_font.h"
#include "tile.h"
#include "window.h"
#include "worldmap.h"
#include "config.h"
#include "sfall_arrays.h"
#include "interface.h"
#include "automap.h"
#include "game.h"
#include "inventory.h"
#include "game_dialog.h"
#include "pipboy.h"
#include "worldmap.h"
#include "character_editor.h"
#include "skilldex.h"

namespace fallout {

typedef void(MetaruleHandler)(Program* program, int args);

// Simplified cousin of `SfallMetarule` from Sfall.
typedef struct MetaruleInfo {
    const char* name;
    MetaruleHandler* handler;
    int minArgs;
    int maxArgs;
} MetaruleInfo;

static void mf_car_gas_amount(Program* program, int args);
static void mf_combat_data(Program* program, int args);
static void mf_critter_inven_obj2(Program* program, int args);
static void mf_dialog_obj(Program* program, int args);
static void mf_get_cursor_mode(Program* program, int args);
static void mf_get_flags(Program* program, int args);
static void mf_get_object_data(Program* program, int args);
static void mf_get_text_width(Program* program, int args);
static void mf_intface_redraw(Program* program, int args);
static void mf_loot_obj(Program* program, int args);
static void mf_metarule_exist(Program* program, int args);
static void mf_outlined_object(Program* program, int args);
static void mf_set_cursor_mode(Program* program, int args);
static void mf_set_flags(Program* program, int args);
static void mf_set_ini_setting(Program* program, int args);
static void mf_get_ini_setting(Program* program, int args);
static void mf_get_window_attribute(Program* program, int args);
static void mf_set_outline(Program* program, int args);
static void mf_show_window(Program* program, int args);
static void mf_tile_refresh_display(Program* program, int args);
static void mf_display_stats(Program* program, int args);
static void mf_inventory_redraw(Program* program, int args);

constexpr MetaruleInfo kMetarules[] = {
    { "car_gas_amount", mf_car_gas_amount, 0, 0 },
    { "combat_data", mf_combat_data, 0, 0 },
    { "critter_inven_obj2", mf_critter_inven_obj2, 2, 2 },
    { "dialog_obj", mf_dialog_obj, 0, 0 },
    { "get_cursor_mode", mf_get_cursor_mode, 0, 0 },
    { "get_flags", mf_get_flags, 1, 1 },
    { "get_object_data", mf_get_object_data, 2, 2 },
    { "get_text_width", mf_get_text_width, 1, 1 },
    { "intface_redraw", mf_intface_redraw, 0, 1 },
    { "loot_obj", mf_loot_obj, 0, 0 },
    { "metarule_exist", mf_metarule_exist, 1, 1 },
    { "outlined_object", mf_outlined_object, 0, 0 },
    { "set_cursor_mode", mf_set_cursor_mode, 1, 1 },
    { "set_flags", mf_set_flags, 2, 2 },
    { "set_ini_setting", mf_set_ini_setting, 2, 2 },
    { "get_ini_section", mf_get_ini_setting, 2, 2 },
    { "get_window_attribute", mf_get_window_attribute, 1, 2 },
    { "set_outline", mf_set_outline, 2, 2 },
    { "show_window", mf_show_window, 0, 1 },
    { "display_stats", mf_display_stats, 0, 0 },
    { "inventory_redraw", mf_inventory_redraw, 0, 1 },
    { "tile_refresh_display", mf_tile_refresh_display, 0, 0 },
};

constexpr int kMetarulesMax = sizeof(kMetarules) / sizeof(kMetarules[0]);

void mf_car_gas_amount(Program* program, int args)
{
    programStackPushInteger(program, wmCarGasAmount());
}

void mf_combat_data(Program* program, int args)
{
    if (isInCombat()) {
        programStackPushPointer(program, combat_get_data());
    } else {
        programStackPushPointer(program, nullptr);
    }
}

void mf_critter_inven_obj2(Program* program, int args)
{
    int slot = programStackPopInteger(program);
    Object* obj = static_cast<Object*>(programStackPopPointer(program));

    switch (slot) {
    case 0:
        programStackPushPointer(program, critterGetArmor(obj));
        break;
    case 1:
        programStackPushPointer(program, critterGetItem2(obj));
        break;
    case 2:
        programStackPushPointer(program, critterGetItem1(obj));
        break;
    case -2:
        programStackPushInteger(program, obj->data.inventory.length);
        break;
    default:
        programFatalError("mf_critter_inven_obj2: invalid type");
    }
}

void mf_dialog_obj(Program* program, int args)
{
    if (GameMode::isInGameMode(GameMode::kDialog)) {
        programStackPushPointer(program, gGameDialogSpeaker);
    } else {
        programStackPushPointer(program, nullptr);
    }
}

void mf_get_cursor_mode(Program* program, int args)
{
    programStackPushInteger(program, gameMouseGetMode());
}

void mf_get_flags(Program* program, int args)
{
    Object* object = static_cast<Object*>(programStackPopPointer(program));
    programStackPushInteger(program, object->flags);
}

void mf_get_object_data(Program* program, int args)
{
    void* ptr = programStackPopPointer(program);
    size_t offset = static_cast<size_t>(programStackPopInteger(program));

    if (offset % 4 != 0) {
        programFatalError("mf_get_object_data: bad offset %d", offset);
    }

    int value = *reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(ptr) + offset);
    programStackPushInteger(program, value);
}

void mf_get_text_width(Program* program, int args)
{
    const char* string = programStackPopString(program);
    programStackPushInteger(program, fontGetStringWidth(string));
}

void mf_intface_redraw(Program* program, int args)
{
    if (args == 0) {
        interfaceBarRefresh();
    } else {
        // TODO: Incomplete.
        programFatalError("mf_intface_redraw: not implemented");
    }

    programStackPushInteger(program, -1);
}

void mf_loot_obj(Program* program, int args)
{
    if (GameMode::isInGameMode(GameMode::kInventory)) {
        programStackPushPointer(program, inven_get_current_target_obj());
    } else {
        programStackPushPointer(program, nullptr);
    }
}

void mf_metarule_exist(Program* program, int args)
{
    const char* metarule = programStackPopString(program);

    for (int index = 0; index < kMetarulesMax; index++) {
        if (strcmp(kMetarules[index].name, metarule) == 0) {
            programStackPushInteger(program, 1);
            return;
        }
    }

    programStackPushInteger(program, 0);
}

void mf_outlined_object(Program* program, int args)
{
    programStackPushPointer(program, gmouse_get_outlined_object());
}

void mf_set_cursor_mode(Program* program, int args)
{
    int mode = programStackPopInteger(program);
    gameMouseSetMode(mode);
    programStackPushInteger(program, -1);
}

void mf_set_flags(Program* program, int args)
{
    int flags = programStackPopInteger(program);
    Object* object = static_cast<Object*>(programStackPopPointer(program));

    object->flags = flags;

    programStackPushInteger(program, -1);
}

enum WinNameType {
    _Inventory = 0, // any inventory window (player/loot/use/barter)
    Dialog = 1,
    PipBoy = 2,
    WorldMap = 3,
    IfaceBar = 4, // the interface bar
    Character = 5,
    Skilldex = 6,
    EscMenu = 7, // escape menu
    Automap = 8,
    // TODO
    DialogView,
    DialogPanel,
    MemberPanel,

    // Inventory types
    Inven = 50, // player inventory
    Loot = 51,
    Use = 53,
    Barter = 54
};

Window* GetWindow(long winType)
{
    long winID = 0;
    switch (winType) {
    case WinNameType::_Inventory:
        if (GameMode::getCurrentGameMode() & (GameMode::kInventory | GameMode::kUseOn | GameMode::kLoot | GameMode::kBarter)) winID = gInventoryWindow;
        break;
    case WinNameType::Dialog:
        if (GameMode::getCurrentGameMode() & GameMode::kDialog) winID = gGameDialogBackgroundWindow;
        break;
    case WinNameType::PipBoy:
        if (GameMode::getCurrentGameMode() & GameMode::kPipboy) winID = gPipboyWindow;
        break;
    case WinNameType::WorldMap:
        if (GameMode::getCurrentGameMode() & GameMode::kWorldmap) winID = wmBkWin;
        break;
    case WinNameType::IfaceBar:
        winID = gInterfaceBarWindow;
        break;
    case WinNameType::Character:
        if (GameMode::getCurrentGameMode() & GameMode::kEditor) winID = gCharacterEditorWindow;
        break;
    case WinNameType::Skilldex:
        if (GameMode::getCurrentGameMode() & GameMode::kSkilldex) winID = gSkilldexWindow;
        break;
    case WinNameType::Automap:
        if (GameMode::getCurrentGameMode() & GameMode::kAutomap) winID = automapWindow;
        break;
    default:
        return (Window*)(-1); // unsupported type
    }
    return (winID > 0) ? windowGetWindow(winID) : nullptr;
}

void mf_get_window_attribute(Program* program, int args)
{
    int v0 = programStackPopInteger(program);
    int v1 = programStackPopInteger(program);

    Window* win = GetWindow(v0);
    if (win == nullptr) {
        if (v1 != 0) {
            debugPrint("%s() - failed to get the interface window.", "mf_get_window_attribute");
            programStackPushInteger(program, -1);
        }
        return;
    }
    if ((long)win == -1) {
        debugPrint("%s() - invalid window type number.", "mf_get_window_attribute");
        programStackPushInteger(program, -1);
        return;
    }
    long result = 0;
    switch (v1) {
    case -1: // rectangle map.left map.top map.right map.bottom
    {
        result = CreateTempArray(-1, 0); // associative
        ProgramValue key1;
        key1.opcode = VALUE_TYPE_DYNAMIC_STRING;
        key1.integerValue = programPushString(program, "left");

        ProgramValue key2;
        key2.opcode = VALUE_TYPE_DYNAMIC_STRING;
        key2.integerValue = programPushString(program, "top");

        ProgramValue key3;
        key3.opcode = VALUE_TYPE_DYNAMIC_STRING;
        key3.integerValue = programPushString(program, "right");

        ProgramValue key4;
        key4.opcode = VALUE_TYPE_DYNAMIC_STRING;
        key4.integerValue = programPushString(program, "bottom");

        SetArray(result, key1, ProgramValue(win->rect.left), false, program);
        SetArray(result, key2, ProgramValue(win->rect.top), false, program);
        SetArray(result, key3, ProgramValue(win->rect.right), false, program);
        SetArray(result, key4, ProgramValue(win->rect.bottom), false, program);
    } break;
    case 0: // check if window exists
        result = 1;
        break;
    case 1:
        result = win->rect.left;
        break;
    case 2:
        result = win->rect.top;
        break;
    case 3:
        result = win->width;
        break;
    case 4:
        result = win->height;
        break;
    }

    programStackPushInteger(program, result);
}

void mf_inventory_redraw(Program* program, int args)
{
    int mode;
    int loopFlag = GameMode::getCurrentGameMode() & (GameMode::kInventory | GameMode::kUseOn | GameMode::kLoot | GameMode::kBarter);
    switch (loopFlag) {
    case GameMode::kInventory:
        mode = 0;
        break;
    case GameMode::kUseOn:
        mode = 1;
        break;
    case GameMode::kLoot:
        mode = 2;
        break;
    case GameMode::kBarter:
        mode = 3;
        break;
    default:
        return;
    }
    long redrawSide = (args > 0) ? programStackPopInteger(program) : -1; // -1 - both
    if (redrawSide <= 0) {
        redrawInventory(1, mode);
    }
    if (redrawSide && mode >= 2) {
        redrawInventory(2, mode);
    }
}


void mf_display_stats(Program* program, int args)
{
     if (GameMode::getCurrentGameMode() & GameMode::kInventory){
         redrawInventory(0,0);
     }
}

void mf_get_ini_setting(Program* program, int args)
{
    char tmp[200];
    const char* string1 = programStackPopString(program);
    const char* sectionName = programStackPopString(program);
    sprintf(tmp, "%s|%s|test", string1, sectionName);

    ArrayId id = CreateTempArray(-1, 0);

    Config config;
    if (!sfall_ini_get(tmp, &config)) {
        programStackPushInteger(program, id);
        return;
    }

    int sectionIndex = dictionaryGetIndexByKey(&config, sectionName);
    if (sectionIndex == -1) {
        programStackPushInteger(program, id);
        return;
    }

    DictionaryEntry* sectionEntry = &(config.entries[sectionIndex]);
    ConfigSection* section = (ConfigSection*)sectionEntry->value;

    for (int i = 0; i < section->entriesLength; i++) {
        char* a = section->entries[i].key;
        char** b = (char**)(section->entries[i].value);

        ProgramValue key;
        key.opcode = VALUE_TYPE_DYNAMIC_STRING;
        key.integerValue = programPushString(program, a);

        ProgramValue val;
        val.opcode = VALUE_TYPE_DYNAMIC_STRING;
        val.integerValue = programPushString(program, *b);

        SetArray(id, key, val, false, program);
    }

    configFree(&config);
    programStackPushInteger(program, id);
}

void mf_set_ini_setting(Program* program, int args)
{
    ProgramValue value = programStackPopValue(program);
    const char* triplet = programStackPopString(program);

    if (value.isString()) {
        const char* stringValue = programGetString(program, value.opcode, value.integerValue);
        if (!sfall_ini_set_string(triplet, stringValue)) {
            debugPrint("set_ini_setting: unable to write '%s' to '%s'",
                stringValue,
                triplet);
        }
    } else {
        int integerValue = value.asInt();
        if (!sfall_ini_set_int(triplet, integerValue)) {
            debugPrint("set_ini_setting: unable to write '%d' to '%s'",
                integerValue,
                triplet);
        }
    }

    programStackPushInteger(program, -1);
}

void mf_set_outline(Program* program, int args)
{
    int outline = programStackPopInteger(program);
    Object* object = static_cast<Object*>(programStackPopPointer(program));
    object->outline = outline;
    programStackPushInteger(program, -1);
}

void mf_show_window(Program* program, int args)
{
    if (args == 0) {
        _windowShow();
    } else if (args == 1) {
        const char* windowName = programStackPopString(program);
        if (!_windowShowNamed(windowName)) {
            debugPrint("show_window: window '%s' is not found", windowName);
        }
    }

    programStackPushInteger(program, -1);
}

void mf_tile_refresh_display(Program* program, int args)
{
    tileWindowRefresh();
    programStackPushInteger(program, -1);
}

void sfall_metarule(Program* program, int args)
{
    static ProgramValue values[6];

    for (int index = 0; index < args; index++) {
        values[index] = programStackPopValue(program);
    }

    const char* metarule = programStackPopString(program);

    for (int index = 0; index < args; index++) {
        programStackPushValue(program, values[index]);
    }

    int metaruleIndex = -1;
    for (int index = 0; index < kMetarulesMax; index++) {
        if (strcmp(kMetarules[index].name, metarule) == 0) {
            metaruleIndex = index;
            break;
        }
    }

    if (metaruleIndex == -1) {
        programFatalError("op_sfall_func: '%s' is not implemented", metarule);
    }

    if (args < kMetarules[metaruleIndex].minArgs || args > kMetarules[metaruleIndex].maxArgs) {
        programFatalError("op_sfall_func: '%s': invalid number of args", metarule);
    }

    kMetarules[metaruleIndex].handler(program, args);
}

} // namespace fallout
