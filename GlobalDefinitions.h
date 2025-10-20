#pragma once
#include "PatternScan.h"

class CSequenceManager;
class CActionMotionManager;
class CFileMotionProperty;
class CFighterCommandManager;

CSequenceManager** SequenceManager = (CSequenceManager**)resolve_relative_addr(PatternScan("48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ? 48 63 88"), 7);
CActionMotionManager** ActionMotionManager = (CActionMotionManager**)resolve_relative_addr(PatternScan("48 8B 0D ? ? ? ? 4C 8D 4D ? C5 FA 11 45"), 7);
CFighterCommandManager** FighterCommandManager = (CFighterCommandManager**)resolve_relative_addr(PatternScan("48 8B 0D ? ? ? ? 48 8D 54 24 ? E8 ? ? ? ? 8B 5C 24"), 7);

BYTE* encounter_table_player_check_addr = (BYTE*)PatternScan("0F 84 ? ? ? ? 83 E9 ? 74 ? 83 F9 ? 0F 85 ? ? ? ? 89 8B");
char* szBattlePlayerKiryu = (char*)resolve_relative_addr(PatternScan("4C 8D 05 ? ? ? ? EB ? 4C 8D 05 ? ? ? ? EB ? 4C 8D 05 ? ? ? ? EB ? 4C 8D 05 ? ? ? ? 8B 05"), 7);
char* szHaruka = (char*)resolve_relative_addr(PatternScan("48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 48 8D 0D"), 7);
char* szEncountTable2 = (char*)PatternScan("64 61 74 61 2F 73 74 61 79 2F 45 6E 63 6F 75 6E 74 54 61 62 6C 65 5F 32 2E 62 69 6E 00");
char* szEncountPrizeTable2 = (char*)PatternScan("64 61 74 61 2F 73 74 61 79 2F 45 6E 63 6F 75 6E 74 50 72 69 7A 65 54 61 62 6C 65 5F 32 2E 62 69 6E 00");

//Address that will be replaced with a reference to haruka's realtime combat moveset
//What if i told you this terrible pattern works in both GOG and Steam?
//It probably wont on MS Store. Surely it will fail that time, right?
BYTE* harukaMovesetReference1 = PatternScan("4C 8D 05 ? ? ? ? EB ? 48 8B 43 ? 4C 8D 05 ? ? ? ? 48 8B 88 ? ? ? ? 81 49 ? ? ? ? ? EB ? 48 8B 43 ? 4C 8D 05 ? ? ? ? 48 8B 88 ? ? ? ? 81 49 ? ? ? ? ? EB ? 4C 8D 05 ? ? ? ? 33 D2 48 8B CB E8 ? ? ? ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 48 8B CB 48 83 C4 ? 5B E9 ? ? ? ?");
BYTE origHarukaMovesetReference1Bytes[7];

//Instead of playing the smoking animation, we'll patch the game
//to play a different animation for Haruka-chan
BYTE* tabakoPatchLocation = PatternScan("? ? 90 00 00 00 93 28");
BYTE origTabakoGmtBytes[10];