#pragma once
#include "PatternScan.h"

class CSequenceManager;
class CActionMotionManager;
class CFileMotionProperty;
class CFighterCommandManager;
class CActionFighterManager;
class CActionCtrlTypeManager;

CSequenceManager** SequenceManager = (CSequenceManager**)resolve_relative_addr(PatternScan("48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ? 48 63 88"), 7);
CActionMotionManager** ActionMotionManager = (CActionMotionManager**)resolve_relative_addr(PatternScan("48 8B 0D ? ? ? ? 4C 8D 4D ? C5 FA 11 45"), 7);
CFighterCommandManager** FighterCommandManager = (CFighterCommandManager**)resolve_relative_addr(PatternScan("48 8B 0D ? ? ? ? 48 8D 54 24 ? E8 ? ? ? ? 8B 5C 24"), 7);
CActionFighterManager** ActionFighterManager = (CActionFighterManager**)(resolve_relative_addr((PatternScan("48 8B 3D ? ? ? ? C5 F8 29 74 24 50")), 7));
CActionCtrlTypeManager** ActionCtrlTypeManager = (CActionCtrlTypeManager**)(resolve_relative_addr((PatternScan("48 8B 05 ? ? ? ? 83 B8 ? ? ? ? ? 0F 84 ? ? ? ? 0F BA E1")), 7));

BYTE* encounter_table_player_check_addr = (BYTE*)PatternScan("0F 84 ? ? ? ? 83 E9 ? 74 ? 83 F9 ? 0F 85 ? ? ? ? 89 8B");
char* szBattlePlayerKiryu = (char*)resolve_relative_addr(PatternScan("4C 8D 05 ? ? ? ? EB ? 4C 8D 05 ? ? ? ? EB ? 4C 8D 05 ? ? ? ? EB ? 4C 8D 05 ? ? ? ? 8B 05"), 7);
char* szHaruka = (char*)resolve_relative_addr(PatternScan("48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 48 8D 0D"), 7);

//bizzare haruka model string that has her at c_cm instead of c_aw, used by colliseum.
char* szCmHaruka = (char*)PatternScan("63 5F 63 6D 5F 68 61 72 75 6B 61 00 00 00 00 00");
char* szEncountTable2 = (char*)PatternScan("64 61 74 61 2F 73 74 61 79 2F 45 6E 63 6F 75 6E 74 54 61 62 6C 65 5F 32 2E 62 69 6E 00");
char* szEncountPrizeTable2 = (char*)PatternScan("64 61 74 61 2F 73 74 61 79 2F 45 6E 63 6F 75 6E 74 50 72 69 7A 65 54 61 62 6C 65 5F 32 2E 62 69 6E 00");

//Replaced with HARUKA_TOUGI_NAME during coliseum
char** szNameShinadaPtr = (char**)resolve_relative_addr(PatternScan("48 8B 15 ? ? ? ? 83 FF"), 7);
char* szNameShinada = *szNameShinadaPtr;

//Address that will be replaced with a reference to haruka's realtime combat moveset
//What if i told you this terrible pattern works in both GOG and Steam?
//It probably wont on MS Store. Surely it will fail that time, right?
BYTE* harukaMovesetReference1 = PatternScan("4C 8D 05 ? ? ? ? EB ? 48 8B 43 ? 4C 8D 05 ? ? ? ? 48 8B 88 ? ? ? ? 81 49 ? ? ? ? ? EB ? 48 8B 43 ? 4C 8D 05 ? ? ? ? 48 8B 88 ? ? ? ? 81 49 ? ? ? ? ? EB ? 4C 8D 05 ? ? ? ? 33 D2 48 8B CB E8 ? ? ? ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 48 8B CB 48 83 C4 ? 5B E9 ? ? ? ?");
BYTE origHarukaMovesetReference1Bytes[7];

//Instead of playing the smoking animation, we'll patch the game
//to play a different animation for Haruka-chan
BYTE* tabakoPatchLocation = PatternScan("? ? 90 00 00 00 93 28");
BYTE origTabakoGmtBytes[10];

LPVOID fighterModeBattleStartConstructorFuncAddr;
LPVOID fighterModePlayerDeathGmtFuncAddr = PatternScan("48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 83 C1 ? E8 ? ? ? ? 48 8B C8 45 33 C0");
LPVOID fighterModePlayerDeathGmtAddr = PatternScan("BA ? ? ? ? E8 ? ? ? ? 48 8B CF 48 8B 5C 24");
LPVOID tougiGetPlayerIDAddr = PatternScan("48 83 EC ? E8 ? ? ? ? 85 C0 75 ? 33 C9 8B C1");
LPVOID createHumanPlayerAddr = PatternScan("48 89 5C 24 ? 55 56 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 49 8B D8");
LPVOID getCtrlTypeForFighterAddr;

BYTE origPlayerDeathGMTBytes[5];