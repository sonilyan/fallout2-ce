
#include "HeroAppearance.h"
#include "sfall_global_vars.h"
#include "stat.h"
#include "object.h"
#include "proto_types.h"
#include "animation.h"
#include "xfile.h"
#include "art.h"
#include "tile.h"
#include "debug.h"

#ifdef _WIN32
#include <filesystem>
#else
#include "file_find.h"
#include "unistd.h"
#endif

namespace fallout {

XBase* heroPathPtr[2] = { nullptr, nullptr };
XBase* racePathPtr[2] = { nullptr, nullptr };


typedef bool(filefunc)(char *path);

filefunc* file_existed;
filefunc* file_is_directory;

#ifdef _WIN32
bool win32_file_existed(char* path)
{
    return std::filesystem::exists(path);
}

bool win32_file_is_directory(char* path)
{
    return std::filesystem::is_directory(path);
}
#else
bool _file_existed(char* path)
{
    int access_result = access(path, F_OK);
    if (access_result == -1) {
        return false;
    } else {
        return true;
    }
}

bool _file_is_directory(char* path)
{
    DIR *asd = opendir(path);
    if(asd == nullptr)
        return false;

    closedir(asd);
    return true;
}
#endif

const char* appearancePathFmt = "Appearance\\h%cR%02dS%02d%s";

static char GetSex()
{
    return critterGetStat(gDude, 34) == GENDER_MALE ? 'M' : 'F';
}

static void SetAppearanceGlobals(int race, int style)
{
    sfall_gl_vars_store("HAp_Race", race);
    sfall_gl_vars_store("HApStyle", style);
}

static void GetAppearanceGlobals(int& race, int& style)
{
    sfall_gl_vars_fetch("HAp_Race", race);
    sfall_gl_vars_fetch("HApStyle", style);
}

XBase* gSecond = nullptr;

int LoadHeroDat(unsigned int race, unsigned int style, bool flush)
{
    if (gSecond == nullptr) {
        gSecond = gXbaseHead->next;
    }

    if (flush) artCacheFlush();

    if (heroPathPtr[1]->dbase) { // unload previous Dats
        dbaseClose(heroPathPtr[1]->dbase);
        heroPathPtr[1]->dbase = nullptr;
        heroPathPtr[1]->isDbase = 0;
    }
    if (racePathPtr[1]->dbase) {
        dbaseClose(racePathPtr[1]->dbase);
        racePathPtr[1]->dbase = nullptr;
        racePathPtr[1]->isDbase = 0;
    }

    const char sex = GetSex();
    bool folderExists = false, heroDatExists = false;
    // check if folder exists for selected appearance
    sprintf(heroPathPtr[0]->path, appearancePathFmt, sex, race, style, "");
    compat_windows_path_to_native(heroPathPtr[0]->path);
    compat_resolve_path(heroPathPtr[0]->path);

    if (file_existed(heroPathPtr[0]->path)) {
        if (file_is_directory(heroPathPtr[0]->path))
            folderExists = true;
    }
    // check if Dat exists for selected appearance
    sprintf(heroPathPtr[1]->path, appearancePathFmt, sex, race, style, ".dat");
    compat_windows_path_to_native(heroPathPtr[1]->path);
    compat_resolve_path(heroPathPtr[1]->path);


    if (file_existed(heroPathPtr[1]->path)) {
        if (!file_is_directory(heroPathPtr[1]->path)) {
            heroPathPtr[1]->dbase = dbaseOpen(heroPathPtr[1]->path);
            heroPathPtr[1]->isDbase = 1;
        }
        if (folderExists)
            heroPathPtr[0]->next = heroPathPtr[1];
        heroDatExists = true;
    } else if (!folderExists) {
        return -1; // no .dat files and folder
    }

    auto heroAppPaths = heroPathPtr[1 - folderExists]; // set path for selected appearance
    heroPathPtr[0 + heroDatExists]->next = gSecond; // heroPathPtr[] >> foPaths

    if (style != 0) {
        bool raceDatExists = false, folderExists = false;
        // check if folder exists for selected race base appearance
        sprintf(racePathPtr[0]->path, appearancePathFmt, sex, race, 0, "");
        compat_windows_path_to_native(racePathPtr[0]->path);
        compat_resolve_path(racePathPtr[0]->path);

        if (file_existed(racePathPtr[0]->path)) {
            if (file_is_directory(racePathPtr[0]->path))
                folderExists = true;
        }

        // check if Dat (or folder) exists for selected race base appearance
        sprintf(racePathPtr[1]->path, appearancePathFmt, sex, race, 0, ".dat");
        compat_windows_path_to_native(racePathPtr[1]->path);
        compat_resolve_path(racePathPtr[1]->path);


        if (file_existed(racePathPtr[1]->path)) {
            if (!file_is_directory(racePathPtr[1]->path)) {
                racePathPtr[1]->dbase = dbaseOpen(racePathPtr[1]->path);
                racePathPtr[1]->isDbase = 1;
            }
            if (folderExists)
                racePathPtr[0]->next = racePathPtr[1];
            raceDatExists = true;
        } else if (!folderExists) {
            return 0; // no .dat files and folder
        }

        heroPathPtr[0 + heroDatExists]->next = racePathPtr[1 - folderExists]; // set path for selected race base appearance
        racePathPtr[0 + raceDatExists]->next = gSecond; // insert racePathPtr in chain path: heroPathPtr[] >> racePathPtr[] >> foPaths
    }

    gXbaseHead->next = heroAppPaths;
    return 0;
}

int currentRaceVal = 0, currentStyleVal = 0;
int critterListSize = 0, critterArraySize = 0;
bool appModEnabled = true;


long AddHeroCritNames()
{ // art_init_
    ArtListDescription& critterArt = getArtListDescription()[OBJ_TYPE_CRITTER];
    critterListSize = critterArt.fileNamesLength / 2;
    if (critterListSize > 2048) {
        return -1;
    }
    critterArraySize = critterListSize * 13;

    char* CritList = critterArt.fileNames; // critter list offset
    char* HeroList = CritList + critterArraySize; // set start of hero critter list after regular critter list

    memset(HeroList, 0, critterArraySize);

    for (int i = 0; i < critterListSize; i++) { // copy critter name list to hero name list
        *HeroList = '_'; // insert a '_' char at the front of new hero critt names. fallout wont load the same name twice
        memcpy(HeroList + 1, CritList, 11);
        HeroList += 13;
        CritList += 13;
    }
    return critterArt.fileNamesLength;
}

void SetHeroArt(bool newArtFlag)
{
    long heroFID = gDude->fid; // get hero FrmID
    int fidBase = heroFID & 0xFFF; // mask out current weapon flag

    if (fidBase > critterListSize) { // check if critter LST index is in Hero range
        if (!newArtFlag) {
            heroFID -= critterListSize; // shift index down into normal critter range
            gDude->fid = heroFID;
        }
    } else if (newArtFlag) {
        heroFID += critterListSize; // shift index up into hero range
        gDude->fid = heroFID; // set new FrmID to hero state struct
    }
}

void DrawPC()
{
    Rect rect;
    objectGetRect(gDude, &rect);
    tileWindowRefreshRect(&rect, gDude->elevation);
}

void LoadHeroAppearance()
{
    if (!appModEnabled) return;

    GetAppearanceGlobals(currentRaceVal, currentStyleVal);
    LoadHeroDat(currentRaceVal, currentStyleVal, true);
    SetHeroArt(true);
    DrawPC();
}

void SetNewCharAppearanceGlobals()
{
    if (!appModEnabled) return;

    if (currentRaceVal > 0 || currentStyleVal > 0) {
        SetAppearanceGlobals(currentRaceVal, currentStyleVal);
    }
}

// op_set_hero_style
void SetHeroStyle(int newStyleVal)
{
    if (!appModEnabled || newStyleVal == currentStyleVal) return;

    if (LoadHeroDat(currentRaceVal, newStyleVal, true) != 0) { // if new style cannot be set
        if (currentRaceVal == 0 && newStyleVal == 0) {
            currentStyleVal = 0; // ignore error if appearance = default
        } else {
            LoadHeroDat(currentRaceVal, currentStyleVal,false); // reload original style
        }
    } else {
        currentStyleVal = newStyleVal;
    }
    SetAppearanceGlobals(currentRaceVal, currentStyleVal);
    DrawPC();
}

// op_set_hero_race
void SetHeroRace(int newRaceVal)
{
    if (!appModEnabled || newRaceVal == currentRaceVal) return;

    if (LoadHeroDat(newRaceVal, 0, true) != 0) { // if new race fails with style at 0
        if (newRaceVal == 0) {
            currentRaceVal = 0;
            currentStyleVal = 0; // ignore if appearance = default
        } else {
            LoadHeroDat(currentRaceVal, currentStyleVal,false); // reload original race & style
        }
    } else {
        currentRaceVal = newRaceVal;
        currentStyleVal = 0;
    }
    SetAppearanceGlobals(currentRaceVal, currentStyleVal); // store new globals
    DrawPC();
}

 
bool raceButtons = true;
bool styleButtons = true;
    
int heroAppearanceMod = -1;

void AdjustHeroArmorArt(int fid)
{
    if (heroAppearanceMod <= 0)
        return;
    if ((fid & 0xF000000) == (OBJ_TYPE_CRITTER << 24)) { //&& !PartyControl::IsNpcControlled()) {
        int fidBase = fid & 0xFFF;
        if (fidBase <= critterListSize) {
            gInventoryWindowDudeFid += critterListSize;
        }
    }
}

void heroAppearanceModinit()
{
#ifdef _WIN32
    file_existed = win32_file_existed;
    file_is_directory = win32_file_is_directory;
#else
    file_existed = _file_existed;
    file_is_directory = _file_is_directory;
#endif

    heroAppearanceMod = 1; //    IniReader::GetConfigInt("Misc", "EnableHeroAppearanceMod", 0);
    if (heroAppearanceMod > 0) {
        debugPrint("Setting up Appearance Char Screen buttons.");

        heroPathPtr[0] = new XBase();
        racePathPtr[0] = new XBase();
        heroPathPtr[0]->path = new char[64];
        racePathPtr[0]->path = new char[64];

        heroPathPtr[1] = new XBase();
        racePathPtr[1] = new XBase();
        heroPathPtr[1]->path = new char[64];
        racePathPtr[1]->path = new char[64];

        // Hero FrmID fix for obj_art_fid/art_change_fid_num script functions
        //if (heroAppearanceMod != 2) {
        //    MakeJump(0x45C5C3, op_obj_art_fid_hack);
        //    HookCall(0x4572BE, op_metarule3_hook);
        //}

        //LoadGameHook::OnAfterNewGame() += []() {
        //  SetNewCharAppearanceGlobals();
        //  LoadHeroAppearance();
        //};
        //Inventory::OnAdjustFid() += AdjustHeroArmorArt;
    }
}
} // namespace fallout
