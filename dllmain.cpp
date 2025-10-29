// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN

#include "buffer.h"
#include <stdio.h>
#include <iostream>
#include <filesystem>

#include "Constants.h"
#include "GlobalDefinitions.h"
#include "FunctionDefinitions.h"
#include "MinHook/MinHook.h"
#pragma comment(lib, "MinHook/libMinHook.x64.lib")

//Commandset that we are going to use based on if we found haruka_battle or not
std::string target_commandset;
char* target_commandset_ptr;

int currentPlayerID = -1;
int currentMissionID = -1;

bool harukaMotionExists = false;

void ApplyEncounterTables()
{
    char buf[256];
    char buf2[256];
    strcpy_s(buf, 256, HARUKA_ENCOUNTER_TABLE);
    strcpy_s(buf2, 256, HARUKA_ENCOUNTER_PRIZE_TABLE);

    const char* encounterTable5Path = parless_get_file_path(buf);
    const char* encounterPrizeTable5Path = parless_get_file_path(buf2);

    //Apply EncounterTable_5 if it exists, otherwise, default to EncounterTable_2 which is Akiyama
    if (std::filesystem::exists(encounterTable5Path))
    {
        strcpy_s(szEncountTable2, strlen(HARUKA_ENCOUNTER_TABLE) + 1, HARUKA_ENCOUNTER_TABLE);
    }

    //Apply EncounterPrizeTable_5 if it exists, otherwise, default to EncounterPrizeTable_2 which is Akiyama
    if (std::filesystem::exists(encounterPrizeTable5Path))
    {
        strcpy_s(szEncountPrizeTable2, strlen(HARUKA_ENCOUNTER_PRIZE_TABLE) + 1, HARUKA_ENCOUNTER_PRIZE_TABLE);
    }
}

void ActionControlTypeManager_DecideBattleStartGMT(void* ctrlTypeMan, char* gmtName)
{
    int mission = get_mission_id();
    hook_original_GetBStartGmtID(ctrlTypeMan, (char*)FALLBACK_BTLST_MOTION);
}

void* fighter_mode_PlayerDeath_DecideGMT(void* fMode)
{
    CActionFighterManager* fman = *ActionFighterManager;

    long* fighter = *(long**)((__int64)fMode + 0x20);
    long* mainPlayer = *(long**)((__int64)fman + 0x290);

    int harukaDeathGmtID = FileMotionProperty_GetGMTID(GetPropertyClass(), HARUKA_GENERIC_DEATH_MOTION);

    //If not haruka or main player (main player comparison so it does not break on Intertwined Fates if Ichiban dies)
    if (currentPlayerID != 4 || harukaDeathGmtID == 0 || fighter != mainPlayer)
    {
        //TODO: write OG byte
        Patch((BYTE*)fighterModePlayerDeathGmtAddr, origPlayerDeathGMTBytes, 5);
    }
    else
    {
        write_int((uintptr_t)fighterModePlayerDeathGmtAddr + 1, harukaDeathGmtID);
    }

    void* result = hook_original_GetPDeathSetGMT(fMode);
    return result;
}

int tougi_get_player_id() {
    
    if (currentPlayerID == 4)
        return 4;

    return hook_original_tougi_get_player_id();
}

int tougi_get_ctrltype_id(int index)
{
    //tougijyo_participant.bin haruka ID
    if (index == 52)
        return 4; //c_cm_haruka... EPIC FAIL!

    return hook_original_tougi_get_ctrltype(index);
}

void create_human_player(void* unk1, __int64 unk2, __int64 unk3)
{
    if (currentPlayerID != 4 || get_mission_id() != 407)
    {
        hook_original_create_human_player(unk1, unk2, unk3);
        return;
    }

    //Colliseum model for Haruka because why not
    //TODO: check if HARUKA_TOUGI_MODEL is a valid model
    CSequenceManager* seqManager = *SequenceManager;
    __int64* missionDataPtr = *(__int64**)((__int64)seqManager + 0x50);
    short* pxdHashPointer = (short*)((__int64)missionDataPtr + 0x398);
    
    //this sucks
    char* imlosingit = (char*)malloc(30);

    *pxdHashPointer = 1790;
    memcpy_s((char*)pxdHashPointer + 2, 30, (void*)imlosingit, 30);
    strcpy_s((char*)pxdHashPointer + 2, strlen(HARUKA_TOUGI_MODEL) + 1, HARUKA_TOUGI_MODEL);

    free(imlosingit);

    hook_original_create_human_player(unk1, unk2, unk3);
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

        if(harukaMotionExists)
            strcpy_s(szBattlePlayerKiryu, strlen(HARUKA_MOTION_PAR) + 1, HARUKA_MOTION_PAR);
        else
            strcpy_s(szBattlePlayerKiryu, strlen(FALLBACK_MOTION_PAR) + 1, FALLBACK_MOTION_PAR);
    }
    
    ApplyEncounterTables();

    strcpy_s(target_commandset_ptr, target_commandset.size() + 1, target_commandset.c_str());

    MH_EnableHook(fighterModePlayerDeathGmtFuncAddr);
    MH_EnableHook(tougiGetPlayerIDAddr);
    MH_EnableHook(createHumanPlayerAddr);
    MH_EnableHook(getCtrlTypeForFighterAddr);
    
    *szNameShinadaPtr = (char*)HARUKA_TOUGI_NAME;
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

    MH_DisableHook(fighterModePlayerDeathGmtFuncAddr);
    MH_DisableHook(tougiGetPlayerIDAddr);
    MH_DisableHook(createHumanPlayerAddr);
    MH_DisableHook(getCtrlTypeForFighterAddr);

    *szNameShinadaPtr = szNameShinada;
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

bool is_battle_mission(int missionID)
{
    return missionID == 400 || missionID == 401 || missionID == 407 || missionID == 408;
}

void on_mission_change(int oldMission, int newMission) 
{
    bool oldMissionWasBattle = is_battle_mission(oldMission);
    bool newMissionIsBattle = is_battle_mission(newMission);

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
    VirtualProtect(szCmHaruka, 32, PAGE_READWRITE, &oldProtect);

    //Fix c_cm_haruka to the appopriate modelname
    strcpy_s(szCmHaruka, strlen(HARUKA_TOUGI_MODEL) + 1, HARUKA_TOUGI_MODEL);

    //Save original values that we are going to restore when Haruka is no longer player 1 or not applicable
    memcpy_s(origHarukaMovesetReference1Bytes, 7, harukaMovesetReference1, 7);
    memcpy_s(origTabakoGmtBytes, 10, tabakoPatchLocation, 10);
    memcpy_s(origPlayerDeathGMTBytes, 5, fighterModePlayerDeathGmtAddr, 5);

    //Enable Weapon Skill tab for Haruka by changing condition to player ID 100 check (impossible)
    BYTE impossiblePlayerID[] = {0x64};
    Patch(PatternScan("83 F8 ? 0F 84 ? ? ? ? E9 ? ? ? ? 45 85 F6") + 2, impossiblePlayerID, 1);
    Patch(PatternScan("83 F8 ? 74 ? 41 B8 ? ? ? ? 48 8B CF") + 2, impossiblePlayerID, 1);
    Patch(PatternScan("83 F8 ? 75 ? 41 FF D0 8B D0 48 8B CB E8 ? ? ? ? BA") + 2, impossiblePlayerID, 1);

    //Create coliseum partner for Haruka (hardcoded for some reason)
    auto ogPartnerTable = PatternScan("48 8D 05 ? ? ? ? 8B 1C 90 E8 ? ? ? ? 48 8B 0D");
    auto partnerBuffer = (int*)AllocateBuffer(ogPartnerTable);

    partnerBuffer[0] = 0xB2;
    partnerBuffer[1] = 0xB4;
    partnerBuffer[2] = 0xB3;
    partnerBuffer[3] = 0xB5;
    partnerBuffer[4] = HARUKA_TOUGI_PARTNER;

    write_relative_addr(ogPartnerTable, (intptr_t)partnerBuffer, 7);

    char harukaMotionBuf[256];
    strcpy_s(harukaMotionBuf, 256, HARUKA_MOTION_PAR_PATH);

    const char* harukaMotionPath = parless_get_file_path(harukaMotionBuf);

    if (std::filesystem::exists(harukaMotionPath))
        harukaMotionExists = true;

    target_commandset_ptr = (char*)AllocateBuffer(szHaruka);

    getCtrlTypeForFighterAddr = (LPVOID)ReadCall2(ReadCall2(PatternScan("E8 ? ? ? ? 48 8B 0D ? ? ? ? 8B D0 E8 ? ? ? ? 48 8D 4C 24")));

    MH_Initialize();
    MH_CreateHook(battleStartDecideGmtFuncAddr, ActionControlTypeManager_DecideBattleStartGMT, (LPVOID*)&hook_original_GetBStartGmtID);
    MH_CreateHook(fighterModePlayerDeathGmtFuncAddr, fighter_mode_PlayerDeath_DecideGMT, (LPVOID*)&hook_original_GetPDeathSetGMT);
    MH_CreateHook(tougiGetPlayerIDAddr, tougi_get_player_id, (LPVOID*)&hook_original_tougi_get_player_id);
    MH_CreateHook(createHumanPlayerAddr, create_human_player, (LPVOID*)&hook_original_create_human_player);
    MH_CreateHook(getCtrlTypeForFighterAddr, tougi_get_ctrltype_id, (LPVOID*)&hook_original_tougi_get_ctrltype);
    MH_EnableHook(battleStartDecideGmtFuncAddr);

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

