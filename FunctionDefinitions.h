#pragma once
#include "PatternScan.h"

inline intptr_t	ReadOffsetValue2(intptr_t address)
{
	intptr_t srcAddr = (intptr_t)address;
	intptr_t dstAddr = srcAddr + 4 + *(int32_t*)srcAddr;
	return dstAddr;
}

template<typename AT>
inline intptr_t	ReadCall2(AT address)
{
	return ReadOffsetValue2((intptr_t)address + 1);
}

inline void Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldProtect;

    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
}

void write_int(uintptr_t addr, int val)
{
	unsigned long OldProtection;
	VirtualProtect((LPVOID)(addr), 4, PAGE_EXECUTE_READWRITE, &OldProtection);

	int* ptr = (int*)addr;
	*ptr = val;

	VirtualProtect((LPVOID)(addr), 4, OldProtection, NULL);
}

inline void write_relative_addr(void* instruction_start, intptr_t target, int instruction_length = 7)
{
	intptr_t instruction_end = (intptr_t)((unsigned long long)instruction_start + instruction_length);
	unsigned int* offset = (unsigned int*)((unsigned long long)instruction_start + (instruction_length - 4));

	int calcOffset = target - instruction_end;
	write_int((intptr_t)offset, calcOffset);
}

typedef const char* (*t_YP_GET_FILE_PATH)(char* path);
typedef int(__fastcall* t_GetPlayerID)();
typedef int(__fastcall* t_GetMissionID)();
typedef unsigned int(__fastcall* FILEMOTIONPROPERTY_GetGMTID)(CFileMotionProperty* fileMotProperty, const char* gmtName);
typedef void(__fastcall* FIGHTERCOMMANDMANAGER_FindCommandsetID)(CFighterCommandManager* fcManager, int& out_id, const char* commandsetName);
typedef void(__fastcall* t_ActionCtrlTypeManager_GetBattleStartGMTID)(void* ctrlTypeMan, char* gmtName);
typedef void* (__fastcall* t_fighter_mode_PlayerDeath_SetGMT)(void* fman);
typedef int (__fastcall* t_tougi_get_player_id)();
typedef int(__fastcall* t_tougi_get_ctrltype)(int index);
typedef void(__fastcall* t_create_human_player)(void* unk1, __int64 unk2, __int64 unk3);

t_YP_GET_FILE_PATH parless_get_file_path = (t_YP_GET_FILE_PATH)GetProcAddress(GetModuleHandle(L"YakuzaParless.asi"), "YP_GET_FILE_PATH");
t_GetPlayerID get_player_id = (t_GetPlayerID)ReadCall2(PatternScan("E8 ? ? ? ? 48 63 8F ? ? ? ? 83 F9"));
t_GetMissionID get_mission_id = (t_GetMissionID)ReadCall2(PatternScan("E8 ? ? ? ? C5 FA 10 35 ? ? ? ? C5 F0 57 C9 BA"));
FILEMOTIONPROPERTY_GetGMTID FileMotionProperty_GetGMTID = (FILEMOTIONPROPERTY_GetGMTID)ReadCall2(PatternScan("E8 ? ? ? ? C5 F8 28 B4 24 ? ? ? ? 48 8B BC 24 ? ? ? ? 48 8B 9C 24"));
FIGHTERCOMMANDMANAGER_FindCommandsetID FighterCommandManager_FindCommandsetID = (FIGHTERCOMMANDMANAGER_FindCommandsetID)ReadCall2(PatternScan("E8 ? ? ? ? 8B 5C 24 ? 89 9C B7"));
t_ActionCtrlTypeManager_GetBattleStartGMTID hook_original_GetBStartGmtID;
t_fighter_mode_PlayerDeath_SetGMT hook_original_GetPDeathSetGMT;
t_tougi_get_player_id hook_original_tougi_get_player_id;
t_tougi_get_ctrltype hook_original_tougi_get_ctrltype;
t_create_human_player hook_original_create_human_player;

CFileMotionProperty* GetPropertyClass()
{
	CFileMotionProperty** file = (CFileMotionProperty**)((__int64)(*ActionMotionManager) + 0x4CF0);
	return *file;
}