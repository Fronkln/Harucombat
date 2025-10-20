// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN

#include "buffer.h"
#include <stdio.h>
#include <iostream>
#include <filesystem>

#include "Constants.h"
#include "GlobalDefinitions.h"
#include "FunctionDefinitions.h"

//Commandset that we are going to use based on if we found haruka_battle or not
std::string target_commandset;
char* target_commandset_ptr;

int currentPlayerID = -1;
int currentMissionID = -1;

void ApplyEncounterTables()
{
    char buf[256];
    char buf2[256];
    strcpy_s(buf, 256, HARUKA_ENCOUNTER_TABLE_PATH);
    strcpy_s(buf2, 256, HARUKA_ENCOUNTER_PRIZE_TABLE_PATH);

    const char* encounterTable5Path = parless_get_file_path(buf);
    const char* encounterPrizeTable5Path = parless_get_file_path(buf2);

    //Apply EncounterTable_5 if it exists, otherwise, default to EncounterTable_2 which is Akiyama
    if (std::filesystem::exists(encounterTable5Path))
    {
        strcpy_s(szEncountTable2, strlen(HARUKA_ENCOUNTER_TABLE_PATH) + 1, HARUKA_ENCOUNTER_TABLE_PATH);
    }

    //Apply EncounterPrizeTable_5 if it exists, otherwise, default to EncounterPrizeTable_2 which is Akiyama
    if (std::filesystem::exists(encounterPrizeTable5Path))
    {
        strcpy_s(szEncountPrizeTable2, strlen(HARUKA_ENCOUNTER_PRIZE_TABLE_PATH) + 1, HARUKA_ENCOUNTER_PRIZE_TABLE_PATH);
    }
}


void on_haruka_start() 
{
    printf("Harucombat time\n");

    BYTE forceLoadEncTablePatch[] = {0xE9, 0xA5, 0x0, 0x0, 0x0, 0x90};

    Patch(encounter_table_player_check_addr, forceLoadEncTablePatch, 6);
    unsigned int harukaPvk2 = FileMotionProperty_GetGMTID(GetPropertyClass(), HARUKA_SPECIAL_PROVOKE);

    //Just default to the taunt animation as a fallback if it even exists at all.
    //If that doesn't exist as well then that's fine. No animation will be played.
    if (harukaPvk2 == 0)
        harukaPvk2 = FileMotionProperty_GetGMTID(GetPropertyClass(), FALLBACK_SPECIAL_PROVOKE);

    write_int((uintptr_t)(tabakoPatchLocation + 6), harukaPvk2);

    CFighterCommandManager* fighterCommandManager = *FighterCommandManager;

    int haruka_commandset_id;
    int default_commandset;

    FighterCommandManager_FindCommandsetID(*FighterCommandManager, default_commandset, "default");
    FighterCommandManager_FindCommandsetID(*FighterCommandManager, haruka_commandset_id, HARUKA_COMMANDSET);

    //haruka_battle does not exist in Fighter Command. Use the fallback.
    if (haruka_commandset_id == default_commandset)
    {
        target_commandset = std::string(FALLBACK_COMMANDSET);
        strcpy_s(szBattlePlayerKiryu, strlen(FALLBACK_MOTION_PAR) + 1, FALLBACK_MOTION_PAR);
    }
    else 
    {
        target_commandset = std::string(HARUKA_COMMANDSET);
        strcpy_s(szBattlePlayerKiryu, strlen(HARUKA_MOTION_PAR) + 1, HARUKA_MOTION_PAR);
    }
    
    ApplyEncounterTables();

    strcpy_s(target_commandset_ptr, target_commandset.size() + 1, target_commandset.c_str());
}

void on_haruka_end() 
{
    //Restore things we modified to how they were
    printf("Harucombat over\n");

    BYTE restoreEncTablePatch[] = { 0x0F, 0x84, 0xA4, 0x0, 0x0, 0x0 };

    Patch(encounter_table_player_check_addr, restoreEncTablePatch, 6);
    Patch(tabakoPatchLocation, origTabakoGmtBytes, 10);

    const char* originalMotionSet = "BattlePlayerKiryu";
    strcpy_s(szBattlePlayerKiryu, strlen(originalMotionSet) + 1, originalMotionSet);


    const char* originalEncountTable = "data/stay/EncountTable_2.bin";
    const char* originalEncountPrizeTable = "data/stay/EncountPrizeTable_2.bin";

    strcpy_s(szEncountTable2, strlen(originalEncountTable) + 1, originalEncountTable);
    strcpy_s(szEncountPrizeTable2, strlen(originalEncountPrizeTable) + 1, originalEncountPrizeTable);
}

void on_combat_start()
{
    //use HARUKA_COMMANDSET or FALLBACK_COMMANDSET when we are in battle
    write_relative_addr(harukaMovesetReference1, (intptr_t)target_commandset_ptr, 7);
}

void on_combat_end()
{
    //use original commandset again
    Patch(harukaMovesetReference1, origHarukaMovesetReference1Bytes, 7);
}


void on_player_id_change(int oldID, int newID) 
{
    if (newID != 4)
        on_haruka_end();
    else if (newID == 4)
        on_haruka_start();
}

void on_mission_change(int oldMission, int newMission) 
{
    bool oldMissionWasBattle = oldMission == 400 || oldMission == 401 || oldMission == 408;
    bool newMissionIsBattle = newMission == 400 || newMission == 401 || newMission == 408;

    if (newMissionIsBattle)
        on_combat_start();
    else
        on_combat_end();
}

DWORD WINAPI ScriptThread(HMODULE hModule)
{
    //Give write permissions to memory addresses we are going to be writing to occassionally
    DWORD oldProtect;
    VirtualProtect(szBattlePlayerKiryu, 32, PAGE_READWRITE, &oldProtect);
    VirtualProtect(szEncountTable2, 128, PAGE_READWRITE, &oldProtect);
    VirtualProtect(szEncountPrizeTable2, 128, PAGE_READWRITE, &oldProtect);
 
    //Save original values that we are going to restore when Haruka is no longer player 1
    memcpy_s(origHarukaMovesetReference1Bytes, 7, harukaMovesetReference1, 7);
    memcpy_s(origTabakoGmtBytes, 10, tabakoPatchLocation, 10);
    target_commandset_ptr = (char*)AllocateBuffer(szHaruka);

    //Main code thread. Checking for player ID and mission ID and invoking events
    //Which makes it all work.
    while (true)
    {
        CSequenceManager* seqManager = *SequenceManager;

        if (seqManager == nullptr)
            continue;

        __int64* missionDataPtr = (__int64*)((__int64)seqManager + 0x50);

        //Yakuza 5 bug, get_mission_id() will crash the game if there is no mission
        if (*missionDataPtr == 0)
            continue;

        int missionID = get_mission_id();

        if (missionID == 2147483646)
            continue;

        if (currentMissionID != missionID)
            on_mission_change(currentMissionID, missionID);

        currentMissionID = missionID;

        int newPlayerID = get_player_id();
        
        if (currentPlayerID != newPlayerID)
            on_player_id_change(currentPlayerID, newPlayerID);

        currentPlayerID = newPlayerID;

        Sleep(10);
    }

    FreeLibraryAndExitThread(hModule, 0);

    return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
        }

        printf("Harucombat: Start\n");
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ScriptThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

