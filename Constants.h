#pragma once
//Look for this commandset in fighter_command.cfc
constexpr auto HARUKA_COMMANDSET = "haruka_battle";
//We'll use this commandset if we can't find HARUKA_COMMANDSET
constexpr auto FALLBACK_COMMANDSET = "akiyama";

//Animation to use in place of the original smoking action other 4 dudes did
constexpr auto HARUKA_SPECIAL_PROVOKE = "P_HAL_ENC_provoke2";
//We'll use this animation if we can't find HARUKA_SPECIAL_PROVOKE
constexpr auto FALLBACK_SPECIAL_PROVOKE = "P_HAL_ENC_provoke";

//Animation par to load if we are using HARUKA_COMMANDSET
constexpr auto HARUKA_MOTION_PAR = "BattlePlayerHaruka";
constexpr auto HARUKA_MOTION_PAR_PATH = "data/motion/BattlePlayerHaruka.par";
//We'll use this animation par if we can't find HARUKA_COMMANDSET
constexpr auto FALLBACK_MOTION_PAR = "BattlePlayerAkiyama";

constexpr auto HARUKA_ENCOUNTER_TABLE = "data/stay/EncountTable_5.bin";
constexpr auto HARUKA_ENCOUNTER_PRIZE_TABLE = "data/stay/EncountPrizeTable_5.bin";

constexpr auto HARUKA_ENCOUNTER_TABLE_PATH = "data/stay/stay_en/EncountTable_5.bin";
constexpr auto HARUKA_ENCOUNTER_PRIZE_TABLE_PATH = "data/stay/stay_en/EncountPrizeTable_5.bin";
