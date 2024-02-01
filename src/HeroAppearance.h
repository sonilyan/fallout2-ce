#ifndef FALLOUT_HERO_H_
#define FALLOUT_HERO_H_

namespace fallout {
void AdjustHeroArmorArt(int fid);
void heroAppearanceModinit();
void SetNewCharAppearanceGlobals();
void LoadHeroAppearance();
long AddHeroCritNames();
extern int critterListSize;
extern int currentRaceVal;
extern int currentStyleVal;
int LoadHeroDat(unsigned int race, unsigned int style, bool flush);
void SetHeroArt(bool newArtFlag);
extern int heroAppearanceMod;
} // namespace fallout
#endif
