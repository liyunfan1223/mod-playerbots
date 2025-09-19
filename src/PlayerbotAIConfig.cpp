/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotAIConfig.h"
#include <iostream>
#include "Config.h"
#include "NewRpgInfo.h"
#include "PlayerbotDungeonSuggestionMgr.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "Talentspec.h"

template <class T>
void LoadList(std::string const value, T& list)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        list.push_back(id);
    }
}

template <class T>
void LoadSet(std::string const value, T& set)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        set.insert(id);
    }
}

template <class T>
void LoadListString(std::string const value, T& list)
{
    std::vector<std::string> strings = split(value, ',');
    for (std::vector<std::string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        std::string const string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

bool PlayerbotAIConfig::Initialize()
{
    LOG_INFO("server.loading", "Initializing AI Playerbots by ike3, based on the original Playerbots by blueboy");

    enabled = sConfigMgr->GetOption<bool>("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        LOG_INFO("server.loading", "AI Playerbots is Disabled in aiplayerbot.conf");
        return false;
    }

    globalCoolDown = sConfigMgr->GetOption<int32>("AiPlayerbot.GlobalCooldown", 1500);
    maxWaitForMove = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxWaitForMove", 5000);
    disableMoveSplinePath = sConfigMgr->GetOption<int32>("AiPlayerbot.DisableMoveSplinePath", 0);
    maxMovementSearchTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxMovementSearchTime", 3);
    expireActionTime = sConfigMgr->GetOption<int32>("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = sConfigMgr->GetOption<int32>("AiPlayerbot.DispelAuraDuration", 7000);
    reactDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ReactDelay", 100);
    dynamicReactDelay = sConfigMgr->GetOption<bool>("AiPlayerbot.DynamicReactDelay", true);
    passiveDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.PassiveDelay", 10000);
    repeatDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.RepeatDelay", 2000);
    errorDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgDelay", 10000);
    sitDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.SitDelay", 30000);
    returnDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.LootDelay", 1000);
    // Buff system
    minBotsForGreaterBuff = sConfigMgr->GetOption<int32>("AiPlayerbot.MinBotsForGreaterBuff", 3);
    rpWarningCooldown     = sConfigMgr->GetOption<int32>("AiPlayerbot.RPWarningCooldown", 30);
    disabledWithoutRealPlayerLoginDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLoginDelay", 30);
    disabledWithoutRealPlayerLogoutDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLogoutDelay", 300);

    farDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = sConfigMgr->GetOption<float>("AiPlayerbot.SightDistance", 75.0f);
    spellDistance = sConfigMgr->GetOption<float>("AiPlayerbot.SpellDistance", 25.0f);
    shootDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = sConfigMgr->GetOption<float>("AiPlayerbot.HealDistance", 25.0f);
    lootDistance = sConfigMgr->GetOption<float>("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FleeDistance", 7.5f);
    aggroDistance = sConfigMgr->GetOption<float>("AiPlayerbot.AggroDistance", 22.0f);
    tooCloseDistance = sConfigMgr->GetOption<float>("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = sConfigMgr->GetOption<float>("AiPlayerbot.MeleeDistance", 0.75f);
    followDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FollowDistance", 1.5f);
    whisperDistance = sConfigMgr->GetOption<float>("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = sConfigMgr->GetOption<float>("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = sConfigMgr->GetOption<float>("AiPlayerbot.RpgDistance", 200.0f);
    grindDistance = sConfigMgr->GetOption<float>("AiPlayerbot.GrindDistance", 75.0f);
    reactDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ReactDistance", 150.0f);

    criticalHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.CriticalHealth", 20);
    lowHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.LowHealth", 45);
    mediumHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.MediumHealth", 65);
    almostFullHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.AlmostFullHealth", 85);
    lowMana = sConfigMgr->GetOption<int32>("AiPlayerbot.LowMana", 15);
    mediumMana = sConfigMgr->GetOption<int32>("AiPlayerbot.MediumMana", 40);
    highMana = sConfigMgr->GetOption<int32>("AiPlayerbot.HighMana", 65);
    autoSaveMana = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoSaveMana", true);
    saveManaThreshold = sConfigMgr->GetOption<int32>("AiPlayerbot.SaveManaThreshold", 60);
    autoAvoidAoe = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoAvoidAoe", true);
    maxAoeAvoidRadius = sConfigMgr->GetOption<float>("AiPlayerbot.MaxAoeAvoidRadius", 15.0f);
    LoadSet<std::set<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.AoeAvoidSpellWhitelist", "50759,57491,13810"),
                              aoeAvoidSpellWhitelist);
    tellWhenAvoidAoe = sConfigMgr->GetOption<bool>("AiPlayerbot.TellWhenAvoidAoe", false);

    randomGearLoweringChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomGearLoweringChance", 0.0f);
    incrementalGearInit = sConfigMgr->GetOption<bool>("AiPlayerbot.IncrementalGearInit", true);
    randomGearQualityLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomGearQualityLimit", 3);
    randomGearScoreLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomGearScoreLimit", 0);

    randomBotMinLevelChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotMinLevelChance", 0.1f);
    randomBotMaxLevelChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotMaxLevelChance", 0.1f);
    randomBotRpgChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotRpgChance", 0.20f);

    iterationsPerTick = sConfigMgr->GetOption<int32>("AiPlayerbot.IterationsPerTick", 100);

    allowAccountBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowAccountBots", true);
    allowGuildBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowGuildBots", true);
    allowTrustedAccountBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowTrustedAccountBots", true);
    disabledWithoutRealPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.DisabledWithoutRealPlayer", false);
    randomBotGuildNearby = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotGuildNearby", false);
    randomBotInvitePlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotInvitePlayer", false);
    inviteChat = sConfigMgr->GetOption<bool>("AiPlayerbot.InviteChat", false);

    randomBotMapsAsString = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<std::vector<uint32>>(randomBotMapsAsString, randomBotMaps);
    probTeleToBankers = sConfigMgr->GetOption<float>("AiPlayerbot.ProbTeleToBankers", 0.25f);
    enableWeightTeleToCityBankers = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableWeightTeleToCityBankers", false);
    weightTeleToStormwind = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToStormwindWeight", 1);
    weightTeleToIronforge = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToIronforgeWeight", 1);
    weightTeleToDarnassus = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToDarnassusWeight", 1);
    weightTeleToExodar = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToExodarWeight", 1);
    weightTeleToOrgrimmar = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToOrgrimmarWeight", 1);
    weightTeleToUndercity = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToUndercityWeight", 1);
    weightTeleToThunderBluff = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToThunderBluffWeight", 1);
    weightTeleToSilvermoonCity = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToSilvermoonCityWeight", 1);
    weightTeleToShattrathCity = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToShattrathCityWeight", 1);
    weightTeleToDalaran = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToDalaranWeight", 1);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotQuestItems",
                                           "6948,5175,5176,5177,5178,16309,12382,13704,11000"),
        randomBotQuestItems);
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotSpellIds", "54197"),
                                  randomBotSpellIds);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.PvpProhibitedZoneIds",
                                           "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,"
                                           "3539,3623,4152,3988,4658,4284,4418,4436,4275,4323,4395,3703,4298,3951"),
        pvpProhibitedZoneIds);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.PvpProhibitedAreaIds",
                                           "976,35,392,2268,4161,4010,4317,4312,3649,3887,3958,3724,4080,3938,3754"),
        pvpProhibitedAreaIds);
    fastReactInBG = sConfigMgr->GetOption<bool>("AiPlayerbot.FastReactInBG", true);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotQuestIds", "7848,3802,5505,6502,7761,10277,10285,11492,13188,13189,24499,24511,24710,24712"),
        randomBotQuestIds);

    LoadSet<std::set<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.DisallowedGameObjects",
                                           "32,41,52,54,57,58,119,263,264,271,272,276,290,321,324,331,333,334,375,759,1165,1166,"
                                           "1560,1562,1571,1594,1610,1617,1618,1619,1620,1621,1622,1623,1624,1628,1667,1673,1723,"
                                           "1727,1731,1732,1733,1734,1735,1736,1759,1760,1765,2039,2040,2041,2042,2043,2044,2045,"
                                           "2046,2047,2054,2055,2068,2084,2086,2087,2554,2560,2653,2656,2689,2690,2691,2707,2708,"
                                           "2709,2710,2712,2714,2716,2717,2718,2724,2739,2740,2741,2742,2743,2744,2842,2843,2844,"
                                           "2845,2847,2848,2849,2850,2852,2855,2857,2858,2866,2867,2883,2891,2892,2893,2907,2910,"
                                           "2912,3236,3237,3238,3239,3240,3290,3640,3642,3646,3658,3659,3660,3661,3662,3685,3689,"
                                           "3690,3691,3693,3694,3695,3703,3704,3705,3706,3707,3715,3719,3724,3725,3726,3727,3729,"
                                           "3730,3763,3764,3767,3768,4096,4149,4406,4608,9630,11713,11714,12654,13359,13360,13872,"
                                           "13873,13891,13949,17155,17282,17783,18036,19015,19016,19017,19018,19019,19020,19022,"
                                           "19283,19284,19541,19542,19595,19596,19597,19598,19602,19603,19861,19868,19869,19870,"
                                           "19871,19872,19873,19877,19903,19904,19905,19906,20691,20725,20726,20727,20807,20920,"
                                           "20925,21052,21277,21530,22245,22246,22550,24798,28024,28604,30854,30855,30856,35252,"
                                           "36738,37098,37099,50935,50936,50937,50982,58369,58595,68865,73941,74447,74448,75293,"
                                           "75295,75296,75298,75299,75300,83763,85562,85563,86492,89634,89635,91138,92013,92420,"
                                           "92423,93192,102984,102985,103600,103628,103664,103711,103713,103815,103820,103821,104564,"
                                           "104569,104574,104575,105169,105170,105171,105172,105174,105175,105176,105569,105578,105579,"
                                           "105581,106318,106319,111095,113757,113768,113769,113770,113771,113772,121264,123214,123309,"
                                           "123310,123329,123330,123331,123332,123333,123848,124388,124389,125477,126049,126158,126260,"
                                           "128293,128308,128403,129127,131978,140911,140971,141596,141853,141931,141979,142076,142088,"
                                           "142140,142141,142142,142143,142144,142145,142181,142184,142185,142186,142187,142188,142191,"
                                           "142344,142477,143980,144053,144054,147557,148499,148506,148513,148514,148515,148516,149036,"
                                           "149480,149481,149482,149483,150079,150080,150081,150082,152094,152095,152608,152618,152620,"
                                           "152621,152622,152631,153123,153239,153451,153453,153454,153464,153468,153469,153470,153471,"
                                           "153472,153473,154357,157936,160836,160845,161495,161521,161526,161527,161557,161752,164658,"
                                           "164659,164660,164661,164662,164778,164779,164780,164781,164798,164910,164958,165658,165738,"
                                           "165739,166863,169243,171938,173232,173234,173266,174728,175165,175166,175207,175245,175324,"
                                           "175329,175330,175331,175334,175382,175384,175385,175404,175407,175487,175488,175565,175566,"
                                           "175588,175606,175628,175629,175708,175785,175802,175886,175888,175889,175891,175892,175893,"
                                           "175928,175949,175950,175965,175966,175970,176089,176092,176112,176116,176142,176143,176145,"
                                           "176150,176151,176188,176189,176206,176207,176208,176209,176213,176215,176224,176248,176249,"
                                           "176304,176307,176308,176309,176325,176327,176344,176356,176484,176485,176486,176487,176582,"
                                           "176583,176584,176586,176587,176588,176589,176630,176634,176635,176636,176637,176638,176639,"
                                           "176640,176641,176642,176643,176645,176751,176752,176753,176785,176793,176944,177264,177287,"
                                           "177388,177464,177726,177747,177750,177784,177785,177789,177790,177792,177794,177804,177805,"
                                           "177806,177844,177926,177964,178084,178085,178087,178104,178105,178106,178144,178184,178185,"
                                           "178186,178195,178204,178244,178246,178325,178464,178504,178553,178609,179486,179487,179488,"
                                           "179489,179490,179491,179492,179493,179494,179496,179497,179498,179499,179501,179516,179526,"
                                           "179528,179532,179533,179545,179547,179548,179553,179565,179703,179826,179828,179908,179914,"
                                           "179915,179922,180164,180165,180166,180167,180168,180215,180216,180228,180229,180370,180371,"
                                           "180372,180373,180374,180392,180435,180436,180501,180510,180511,180512,180523,180600,180690,"
                                           "180691,180754,180905,180917,180921,181053,181085,181098,181107,181108,181109,181110,181133,"
                                           "181138,181139,181140,181151,181166,181238,181239,181248,181249,181270,181271,181275,181276,"
                                           "181277,181278,181279,181280,181281,181283,181372,181385,181555,181556,181557,181569,181589,"
                                           "181590,181593,181594,181620,181626,181627,181628,181629,181637,181644,181645,181665,181671,"
                                           "181674,181675,181680,181681,181683,181686,181687,181696,181697,181746,181768,181770,181771,"
                                           "181798,181800,181802,181804,181833,181841,181843,181845,181846,181847,181848,181854,181871,"
                                           "181872,181873,181874,181875,181876,181891,181892,181893,181894,181897,181898,181916,181963,"
                                           "181981,182011,182031,182050,182053,182054,182063,182069,182095,182116,182119,182122,182127,"
                                           "182128,182139,182166,182184,182185,182199,182256,182258,182265,182355,182520,182532,182541,"
                                           "182542,182581,182583,182584,182599,182600,182601,182606,182797,182798,182799,182804,182936,"
                                           "182937,182938,182940,182941,183043,183044,183045,183046,183050,183385,183394,183395,183396,"
                                           "183397,183441,183767,183813,183814,183934,183935,183945,184031,184075,184077,184115,184121,"
                                           "184122,184123,184304,184310,184433,184443,184465,184466,184478,184504,184560,184589,184607,"
                                           "184689,184716,184726,184727,184728,184740,184741,184744,184793,184795,184796,184810,184811,"
                                           "184812,184813,184815,184834,184849,184859,184860,184869,184870,184947,184948,185015,185032,"
                                           "185119,185128,185130,185152,185168,185169,185182,185200,185201,185220,185224,185225,185226,"
                                           "185233,185302,185497,185541,185557,185877,185881,185911,185915,185939,185954,186266,186272,"
                                           "186273,186301,186302,186329,186390,186397,186423,186427,186450,186463,186587,186591,186595,"
                                           "186607,186618,186619,186632,186648,186662,186679,186684,186729,186733,186748,186828,186830,"
                                           "186831,186832,186885,186886,186912,186938,186946,186950,186954,186955,187022,187023,187026,"
                                           "187027,187032,187033,187039,187264,187333,187347,187367,187372,187381,187577,187659,187660,"
                                           "187661,187670,187671,187673,187677,187683,187684,187685,187686,187687,187689,187697,187885,"
                                           "187886,187897,187898,187899,187900,187901,187902,187980,188015,188016,188017,188018,188113,"
                                           "188120,188131,188140,188164,188236,188237,188238,188260,188262,188263,188345,188351,188359,"
                                           "188432,188441,188442,188461,188489,188499,188501,188502,188503,188518,188530,188600,188601,"
                                           "188646,188650,188658,188659,188670,188671,188675,188676,188677,188691,188699,188702,188703,"
                                           "188705,189288,189291,189293,189295,189298,189306,189973,189976,189978,189979,189980,189981,"
                                           "189983,189992,190032,190169,190170,190171,190172,190173,190174,190175,190176,190189,190208,"
                                           "190223,190399,190447,190453,190454,190455,190459,190461,190462,190463,190464,190465,190466,"
                                           "190467,190468,190469,190470,190471,190472,190473,190474,190476,190477,190478,190479,190480,"
                                           "190481,190482,190483,190484,190500,190533,190534,190537,190540,190541,190542,190543,190578,"
                                           "190584,190586,190633,190660,190691,190702,190717,190718,190719,190720,190778,190836,190937,"
                                           "190938,190947,191019,191084,191133,191180,191181,191182,191209,191210,191303,191349,191519,"
                                           "191530,191531,191532,191533,191534,191537,191543,191567,191568,191576,191708,191770,191773,"
                                           "191780,191781,191782,191783,191814,191815,191843,191844,191845,192058,192081,192171,192172,"
                                           "192187,192188,192189,192190,192191,192192,192243,192536,192556,192676,192693,192773,192788,"
                                           "192818,192823,192824,192825,192826,192827,192828,192908,192909,192910,192911,192912,192941,"
                                           "192942,192943,192944,192945,193059,193091,193092,193196,193197,193199,193200,193201,193402,"
                                           "193403,193404,193405,193406,193560,193561,193603,193610,193767,193792,193793,193943,193944,"
                                           "193945,193946,193996,193997,194126,194213,194340,194423,194424,195022,195036,195037,195274,"
                                           "195344,201367,201592,201710,201778,201872,201873,201874,201875,202080,202081,202082,202083,"
                                           "202177,202178,202179,202180,202238,202239,202240,202241,202336,202947,202948"),
        disallowedGameObjects);
    botAutologin = sConfigMgr->GetOption<bool>("AiPlayerbot.BotAutologin", false);
    randomBotAutologin = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotAutologin", true);
    minRandomBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBots", 50);
    randomBotUpdateInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotUpdateInterval", 20);
    randomBotCountChangeMinInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotCountChangeMinInterval", 30 * MINUTE);
    randomBotCountChangeMaxInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * HOUR);
    minRandomBotInWorldTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotInWorldTime", 2 * HOUR);
    maxRandomBotInWorldTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotInWorldTime", 14 * 24 * HOUR);
    minRandomBotRandomizeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotRandomizeTime", 2 * HOUR);
    maxRandomBotRandomizeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotRandomizeTime", 14 * 24 * HOUR);
    minRandomBotChangeStrategyTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotChangeStrategyTime", 30 * MINUTE);
    maxRandomBotChangeStrategyTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * HOUR);
    minRandomBotReviveTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotReviveTime", MINUTE);
    maxRandomBotReviveTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotReviveTime", 5 * MINUTE);
    minRandomBotTeleportInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotTeleportInterval", 1 * HOUR);
    maxRandomBotTeleportInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotTeleportInterval", 5 * HOUR);
    permanantlyInWorldTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.PermanantlyInWorldTime", 1 * YEAR);
    randomBotTeleportDistance = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleportDistance", 100);
    randomBotsPerInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotsPerInterval", 60);
    minRandomBotsPriceChangeInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * HOUR);
    maxRandomBotsPriceChangeInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * HOUR);
    randomBotJoinLfg = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotJoinLfg", true);

    restrictHealerDPS = sConfigMgr->GetOption<bool>("AiPlayerbot.HealerDPSMapRestriction", false);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RestrictedHealerDPSMaps",
                                             "33,34,36,43,47,48,70,90,109,129,209,229,230,329,349,389,429,1001,1004,"
                                             "1007,269,540,542,543,545,546,547,552,553,554,555,556,557,558,560,585,574,"
                                             "575,576,578,595,599,600,601,602,604,608,619,632,650,658,668,409,469,509,"
                                             "531,532,534,544,548,550,564,565,580,249,533,603,615,616,624,631,649,724"),
        restrictedHealerDPSMaps);

	//////////////////////////// ICC

	EnableICCBuffs = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableICCBuffs", true);

    //////////////////////////// CHAT
    enableBroadcasts = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableBroadcasts", true);
    randomBotTalk = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotTalk", false);
    randomBotEmote = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotEmote", false);
    randomBotSuggestDungeons = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotSuggestDungeons", true);
    randomBotSayWithoutMaster = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotSayWithoutMaster", false);

    // broadcastChanceMaxValue is used in urand(1, broadcastChanceMaxValue) for broadcasts,
    // lowering it will increase the chance, setting it to 0 will disable broadcasts
    // for internal use, not intended to be change by the user
    broadcastChanceMaxValue = enableBroadcasts ? 30000 : 0;

    // all broadcast chances should be in range 1-broadcastChanceMaxValue, value of 0 will disable this particular
    // broadcast setting value to max does not guarantee the broadcast, as there are some internal randoms as well
    broadcastToGuildGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGuildGlobalChance", 30000);
    broadcastToWorldGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToWorldGlobalChance", 30000);
    broadcastToGeneralGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGeneralGlobalChance", 30000);
    broadcastToTradeGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToTradeGlobalChance", 30000);
    broadcastToLFGGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToLFGGlobalChance", 30000);
    broadcastToLocalDefenseGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToLocalDefenseGlobalChance", 30000);
    broadcastToWorldDefenseGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToWorldDefenseGlobalChance", 30000);
    broadcastToGuildRecruitmentGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGuildRecruitmentGlobalChance", 30000);

    broadcastChanceLootingItemPoor = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemPoor", 30);
    broadcastChanceLootingItemNormal =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemNormal", 300);
    broadcastChanceLootingItemUncommon =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemUncommon", 10000);
    broadcastChanceLootingItemRare = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemRare", 20000);
    broadcastChanceLootingItemEpic = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemEpic", 30000);
    broadcastChanceLootingItemLegendary =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemLegendary", 30000);
    broadcastChanceLootingItemArtifact =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemArtifact", 30000);

    broadcastChanceQuestAccepted = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestAccepted", 6000);
    broadcastChanceQuestUpdateObjectiveCompleted =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveCompleted", 300);
    broadcastChanceQuestUpdateObjectiveProgress =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveProgress", 300);
    broadcastChanceQuestUpdateFailedTimer =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateFailedTimer", 300);
    broadcastChanceQuestUpdateComplete =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateComplete", 1000);
    broadcastChanceQuestTurnedIn = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestTurnedIn", 10000);

    broadcastChanceKillNormal = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillNormal", 30);
    broadcastChanceKillElite = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillElite", 300);
    broadcastChanceKillRareelite = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillRareelite", 3000);
    broadcastChanceKillWorldboss = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillWorldboss", 20000);
    broadcastChanceKillRare = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillRare", 10000);
    broadcastChanceKillUnknown = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillUnknown", 100);
    broadcastChanceKillPet = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillPet", 10);
    broadcastChanceKillPlayer = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillPlayer", 30);

    broadcastChanceLevelupGeneric = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupGeneric", 20000);
    broadcastChanceLevelupTenX = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupTenX", 30000);
    broadcastChanceLevelupMaxLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupMaxLevel", 30000);

    broadcastChanceSuggestInstance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestInstance", 5000);
    broadcastChanceSuggestQuest = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestQuest", 10000);
    broadcastChanceSuggestGrindMaterials =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindMaterials", 5000);
    broadcastChanceSuggestGrindReputation =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindReputation", 5000);
    broadcastChanceSuggestSell = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSell", 300);
    broadcastChanceSuggestSomething =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomething", 30000);

    broadcastChanceSuggestSomethingToxic =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomethingToxic", 0);

    broadcastChanceSuggestToxicLinks = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestToxicLinks", 0);
    toxicLinksPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.ToxicLinksPrefix", "gnomes");

    broadcastChanceSuggestThunderfury =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestThunderfury", 1);

    // does not depend on global chance
    broadcastChanceGuildManagement = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceGuildManagement", 30000);
    ////////////////////////////

    toxicLinksRepliesChance = sConfigMgr->GetOption<int32>("AiPlayerbot.ToxicLinksRepliesChance", 30);    // 0-100
    thunderfuryRepliesChance = sConfigMgr->GetOption<int32>("AiPlayerbot.ThunderfuryRepliesChance", 40);  // 0-100
    guildRepliesRate = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildRepliesRate", 100);                 // 0-100
    suggestDungeonsInLowerCaseRandomly =
        sConfigMgr->GetOption<bool>("AiPlayerbot.SuggestDungeonsInLowerCaseRandomly", false);

    ////////////////////////// !CHAT

    randomBotJoinBG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotJoinBG", true);
    randomBotAutoJoinBG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotAutoJoinBG", false);

    randomBotAutoJoinArenaBracket = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinArenaBracket", 7);

    randomBotAutoJoinICBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinICBrackets", "0,1");
    randomBotAutoJoinEYBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinEYBrackets", "0,1,2");
    randomBotAutoJoinAVBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinAVBrackets", "0,1,2,3");
    randomBotAutoJoinABBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinABBrackets", "0,1,2,3,4,5,6");
    randomBotAutoJoinWSBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinWSBrackets", "0,1,2,3,4,5,6,7");

    randomBotAutoJoinBGICCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGICCount", 0);
    randomBotAutoJoinBGEYCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGEYCount", 0);
    randomBotAutoJoinBGAVCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGAVCount", 0);
    randomBotAutoJoinBGABCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGABCount", 0);
    randomBotAutoJoinBGWSCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGWSCount", 0);

    randomBotAutoJoinBGRatedArena2v2Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena2v2Count", 0);
    randomBotAutoJoinBGRatedArena3v3Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena3v3Count", 0);
    randomBotAutoJoinBGRatedArena5v5Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena5v5Count", 0);
    logInGroupOnly = sConfigMgr->GetOption<bool>("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = sConfigMgr->GetOption<bool>("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotMaxLevel", 80);
    if (randomBotMaxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        randomBotMaxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    randomBotLoginAtStartup = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotLoginAtStartup", true);
    randomBotTeleLowerLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleLowerLevel", 1);
    randomBotTeleHigherLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleHigherLevel", 3);
    openGoSpell = sConfigMgr->GetOption<int32>("AiPlayerbot.OpenGoSpell", 6477);

    // Zones for NewRpgStrategy teleportation brackets
    std::vector<uint32> zoneIds = {
        // Classic WoW - Low-level zones
        1, 12, 14, 85, 141, 215, 3430, 3524,
        // Classic WoW - Mid-level zones
        17, 38, 40, 130, 148, 3433, 3525,
        // Classic WoW - High-level zones
        10, 11, 44, 267, 331, 400, 406,
        // Classic WoW - Higher-level zones
        3, 8, 15, 16, 33, 45, 47, 51, 357, 405, 440,
        // Classic WoW - Top-level zones
        4, 28, 46, 139, 361, 490, 618, 1377,
        // The Burning Crusade - Zones
        3483, 3518, 3519, 3520, 3521, 3522, 3523, 4080,
        // Wrath of the Lich King - Zones
        65, 66, 67, 210, 394, 495, 2817, 3537, 3711, 4197
    };

    for (uint32 zoneId : zoneIds)
    {
        std::string setting = "AiPlayerbot.ZoneBracket." + std::to_string(zoneId);
        std::string value = sConfigMgr->GetOption<std::string>(setting, "");

        if (!value.empty())
        {
            size_t commaPos = value.find(',');
            if (commaPos != std::string::npos)
            {
                uint32 minLevel = atoi(value.substr(0, commaPos).c_str());
                uint32 maxLevel = atoi(value.substr(commaPos + 1).c_str());
                zoneBrackets[zoneId] = std::make_pair(minLevel, maxLevel);
            }
        }
    }

    randomChangeMultiplier = sConfigMgr->GetOption<float>("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotCombatStrategies", "");
    randomBotNonCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotNonCombatStrategies", "");
    combatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.CombatStrategies", "+custom::say");
    nonCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.NonCombatStrategies", "+custom::say,+return");
    applyInstanceStrategies = sConfigMgr->GetOption<bool>("AiPlayerbot.ApplyInstanceStrategies", true);

    commandPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.CommandPrefix", "");
    commandSeparator = sConfigMgr->GetOption<std::string>("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = sConfigMgr->GetOption<int32>("AiPlayerbot.CommandServerPort", 8888);
    perfMonEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.PerfMonEnabled", false);

    useGroundMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseGroundMountAtMinLevel", 20);
    useFastGroundMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFastGroundMountAtMinLevel", 40);
    useFlyMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFlyMountAtMinLevel", 60);
    useFastFlyMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFastFlyMountAtMinLevel", 70);

    // stagger bot flightpath takeoff
    delayMin = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMinMs", 350u);
    delayMax = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMaxMs", 5000u);
    gapMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapMs", 200u);
    gapJitterMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapJitterMs", 100u);

    LOG_INFO("server.loading", "Loading TalentSpecs...");

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        if (cls == 10)
        {
            continue;
        }
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            premadeSpecName[cls][spec] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.PremadeSpecGlyph." << cls << "." << spec;
            premadeSpecGlyph[cls][spec] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
            std::vector<std::string> splitSpecGlyph = split(premadeSpecGlyph[cls][spec], ',');
            for (std::string& split : splitSpecGlyph)
            {
                if (split.size() != 0)
                {
                    parsedSpecGlyph[cls][spec].push_back(atoi(split.c_str()));
                }
            }
            for (uint32 level = 0; level < MAX_LEVEL; ++level)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                premadeSpecLink[cls][spec][level] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
                parsedSpecLinkOrder[cls][spec][level] = ParseTempTalentsOrder(cls, premadeSpecLink[cls][spec][level]);
            }
        }
        for (uint32 spec = 0; spec < 3; ++spec)
        {
            for (uint32 points = 0; points < 21; ++points)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeHunterPetLink." << spec << "." << points;
                premadeHunterPetLink[spec][points] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
                parsedHunterPetLinkOrder[spec][points] =
                    ParseTempPetTalentsOrder(spec, premadeHunterPetLink[spec][points]);
            }
        }
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.RandomClassSpecProb." << cls << "." << spec;
            uint32 def;
            if (spec <= 1)
                def = 33;
            else if (spec == 2)
                def = 34;
            else
                def = 0;
            randomClassSpecProb[cls][spec] = sConfigMgr->GetOption<uint32>(os.str().c_str(), def, false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.RandomClassSpecIndex." << cls << "." << spec;
            randomClassSpecIndex[cls][spec] = sConfigMgr->GetOption<uint32>(os.str().c_str(), spec, false);
        }
    }

    botCheats.clear();
    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.BotCheats", "food,taxi,raid"),
                                             botCheats);

    botCheatMask = 0;

    if (std::find(botCheats.begin(), botCheats.end(), "food") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::food;
    if (std::find(botCheats.begin(), botCheats.end(), "taxi") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(botCheats.begin(), botCheats.end(), "gold") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(botCheats.begin(), botCheats.end(), "health") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(botCheats.begin(), botCheats.end(), "mana") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(botCheats.begin(), botCheats.end(), "power") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::power;
    if (std::find(botCheats.begin(), botCheats.end(), "raid") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::raid;

    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.AllowedLogFiles", ""),
                                             allowedLogFiles);
    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.TradeActionExcludedPrefixes", ""),
                                             tradeActionExcludedPrefixes);

    worldBuffs.clear();
    loadWorldBuff();
    LOG_INFO("playerbots", "Loading World Buff Feature...");

    randomBotAccountPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAccountCount", 0);
    deleteRandomBotAccounts = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotAccounts", false);
    randomBotGuildCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotGuildCount", 20);
    deleteRandomBotGuilds = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotGuilds", false);

    guildTaskEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableGuildTasks", true);
    minGuildTaskChangeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskChangeTime", 3 * 24 * 3600);
    maxGuildTaskChangeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskChangeTime", 4 * 24 * 3600);
    minGuildTaskAdvertisementTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskAdvertisementTime", 300);
    maxGuildTaskAdvertisementTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskAdvertisementTime", 12 * 3600);
    minGuildTaskRewardTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskRewardTime", 300);
    maxGuildTaskRewardTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskRewardTime", 3600);
    guildTaskAdvertCleanupTime = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildTaskAdvertCleanupTime", 300);
    guildTaskKillTaskDistance = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildTaskKillTaskDistance", 2000);
    targetPosRecalcDistance = sConfigMgr->GetOption<float>("AiPlayerbot.TargetPosRecalcDistance", 0.1f);

    // cosmetics (by lidocain)
    randomBotShowCloak = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotShowCloak", true);
    randomBotShowHelmet = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotShowHelmet", true);

    // SPP switches
    enableGreet = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableGreet", true);
    summonWhenGroup = sConfigMgr->GetOption<bool>("AiPlayerbot.SummonWhenGroup", true);
    randomBotFixedLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotFixedLevel", false);
    disableRandomLevels = sConfigMgr->GetOption<bool>("AiPlayerbot.DisableRandomLevels", false);
    randomBotRandomPassword = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotRandomPassword", true);
    downgradeMaxLevelBot = sConfigMgr->GetOption<bool>("AiPlayerbot.DowngradeMaxLevelBot", true);
    equipmentPersistence = sConfigMgr->GetOption<bool>("AiPlayerbot.EquipmentPersistence", false);
    equipmentPersistenceLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.EquipmentPersistenceLevel", 80);
    groupInvitationPermission = sConfigMgr->GetOption<int32>("AiPlayerbot.GroupInvitationPermission", 1);
    keepAltsInGroup = sConfigMgr->GetOption<bool>("AiPlayerbot.KeepAltsInGroup", false);
    allowSummonInCombat = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonInCombat", true);
    allowSummonWhenMasterIsDead = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonWhenMasterIsDead", true);
    allowSummonWhenBotIsDead = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonWhenBotIsDead", true);
    reviveBotWhenSummoned = sConfigMgr->GetOption<int32>("AiPlayerbot.ReviveBotWhenSummoned", 1);
    botRepairWhenSummon = sConfigMgr->GetOption<bool>("AiPlayerbot.BotRepairWhenSummon", true);
    autoInitOnly = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoInitOnly", false);
    autoInitEquipLevelLimitRatio = sConfigMgr->GetOption<float>("AiPlayerbot.AutoInitEquipLevelLimitRatio", 1.0);

    maxAddedBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxAddedBots", 40);
    addClassCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AddClassCommand", 1);
    addClassAccountPoolSize = sConfigMgr->GetOption<int32>("AiPlayerbot.AddClassAccountPoolSize", 50);
    maintenanceCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.MaintenanceCommand", 1);
    autoGearCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearCommand", 1);
    autoGearCommandAltBots = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearCommandAltBots", 1);
    autoGearQualityLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearQualityLimit", 3);
    autoGearScoreLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearScoreLimit", 0);

    randomBotXPRate = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotXPRate", 1.0);
    randomBotAllianceRatio = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAllianceRatio", 50);
    randomBotHordeRatio = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotHordeRatio", 50);
    disableDeathKnightLogin = sConfigMgr->GetOption<bool>("AiPlayerbot.DisableDeathKnightLogin", 0);
    limitTalentsExpansion = sConfigMgr->GetOption<bool>("AiPlayerbot.LimitTalentsExpansion", 0);
    botActiveAlone = sConfigMgr->GetOption<int32>("AiPlayerbot.BotActiveAlone", 100);
    BotActiveAloneForceWhenInRadius = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotActiveAloneForceWhenInRadius", 150);
    BotActiveAloneForceWhenInZone = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInZone", 1);
    BotActiveAloneForceWhenInMap = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInMap", 0);
    BotActiveAloneForceWhenIsFriend = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenIsFriend", 1);
    BotActiveAloneForceWhenInGuild = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInGuild", 1);
    botActiveAloneSmartScale = sConfigMgr->GetOption<bool>("AiPlayerbot.botActiveAloneSmartScale", 1);
    botActiveAloneSmartScaleDiffLimitfloor = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitfloor", 50);
    botActiveAloneSmartScaleDiffLimitCeiling = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitCeiling", 200);
    botActiveAloneSmartScaleWhenMinLevel = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMinLevel", 1);
    botActiveAloneSmartScaleWhenMaxLevel = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMaxLevel", 80);

    randombotsWalkingRPG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandombotsWalkingRPG", false);
    randombotsWalkingRPGInDoors = sConfigMgr->GetOption<bool>("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.MinEnchantingBotLevel", 60);
    limitEnchantExpansion = sConfigMgr->GetOption<int32>("AiPlayerbot.LimitEnchantExpansion", 1);
    limitGearExpansion = sConfigMgr->GetOption<int32>("AiPlayerbot.LimitGearExpansion", 1);
    randombotStartingLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandombotStartingLevel", 5);
    enablePeriodicOnlineOffline = sConfigMgr->GetOption<bool>("AiPlayerbot.EnablePeriodicOnlineOffline", false);
    enableRandomBotTrading = sConfigMgr->GetOption<int32>("AiPlayerbot.EnableRandomBotTrading", 1);
    periodicOnlineOfflineRatio = sConfigMgr->GetOption<float>("AiPlayerbot.PeriodicOnlineOfflineRatio", 2.0);
    gearscorecheck = sConfigMgr->GetOption<bool>("AiPlayerbot.GearScoreCheck", false);
    randomBotPreQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.PreQuests", true);

    // SPP automation
    freeMethodLoot = sConfigMgr->GetOption<bool>("AiPlayerbot.FreeMethodLoot", false);
    lootRollLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.LootRollLevel", 1);
    autoPickReward = sConfigMgr->GetOption<std::string>("AiPlayerbot.AutoPickReward", "yes");
    autoEquipUpgradeLoot = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoEquipUpgradeLoot", true);
    equipUpgradeThreshold = sConfigMgr->GetOption<float>("AiPlayerbot.EquipUpgradeThreshold", 1.1f);
    twoRoundsGearInit = sConfigMgr->GetOption<bool>("AiPlayerbot.TwoRoundsGearInit", false);
    syncQuestWithPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncQuestWithPlayer", true);
    syncQuestForPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncQuestForPlayer", false);
    dropObsoleteQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.DropObsoleteQuests", true);
    autoTrainSpells = sConfigMgr->GetOption<std::string>("AiPlayerbot.AutoTrainSpells", "yes");
    autoPickTalents = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoPickTalents", true);
    autoUpgradeEquip = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoUpgradeEquip", false);
    hunterWolfPet = sConfigMgr->GetOption<int32>("AiPlayerbot.HunterWolfPet", 0);
    defaultPetStance = sConfigMgr->GetOption<int32>("AiPlayerbot.DefaultPetStance", 1);
    petChatCommandDebug = sConfigMgr->GetOption<bool>("AiPlayerbot.PetChatCommandDebug", 0);
    autoLearnTrainerSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoLearnTrainerSpells", true);
    autoLearnQuestSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoLearnQuestSpells", false);
    autoTeleportForLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoTeleportForLevel", false);
    autoDoQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoDoQuests", true);
    enableNewRpgStrategy = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableNewRpgStrategy", true);

    RpgStatusProbWeight[RPG_WANDER_RANDOM] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderRandom", 15);
    RpgStatusProbWeight[RPG_WANDER_NPC] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderNpc", 20);
    RpgStatusProbWeight[RPG_GO_GRIND] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoGrind", 15);
    RpgStatusProbWeight[RPG_GO_CAMP] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoCamp", 10);
    RpgStatusProbWeight[RPG_DO_QUEST] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.DoQuest", 60);
    RpgStatusProbWeight[RPG_TRAVEL_FLIGHT] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.TravelFlight", 15);
    RpgStatusProbWeight[RPG_REST] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.Rest", 5);

    syncLevelWithPlayers = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncLevelWithPlayers", false);
    randomBotGroupNearby = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotGroupNearby", true);

    // arena
    randomBotArenaTeam2v2Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam2v2Count", 10);
    randomBotArenaTeam3v3Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam3v3Count", 10);
    randomBotArenaTeam5v5Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam5v5Count", 5);
    deleteRandomBotArenaTeams = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotArenaTeams", false);
    randomBotArenaTeamMaxRating = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeamMaxRating", 2000);
    randomBotArenaTeamMinRating = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeamMinRating", 1000);

    selfBotLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.SelfBotLevel", 1);

    RandomPlayerbotFactory::CreateRandomBots();
    if (World::IsStopped())
    {
        return true;
    }

    // Assign account types after accounts are created
    sRandomPlayerbotMgr->AssignAccountTypes();

    if (sPlayerbotAIConfig->enabled)
    {
        sRandomPlayerbotMgr->Init();
    }

    sRandomItemMgr->Init();
    sRandomItemMgr->InitAfterAhBot();
    sPlayerbotTextMgr->LoadBotTexts();
    sPlayerbotTextMgr->LoadBotTextChance();
    PlayerbotFactory::Init();

    AiObjectContext::BuildAllSharedContexts();

    if (sPlayerbotAIConfig->randomBotSuggestDungeons)
    {
        sPlayerbotDungeonSuggestionMgr->LoadDungeonSuggestions();
    }

    excludedHunterPetFamilies.clear();
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.ExcludedHunterPetFamilies", ""), excludedHunterPetFamilies);

    LOG_INFO("server.loading", "---------------------------------------");
    LOG_INFO("server.loading", "        AI Playerbots initialized       ");
    LOG_INFO("server.loading", "---------------------------------------");

    return true;
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(randomBotQuestItems.begin(), randomBotQuestItems.end(), id) != randomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsPvpProhibited(uint32 zoneId, uint32 areaId)
{
    return IsInPvpProhibitedZone(zoneId) || IsInPvpProhibitedArea(areaId) || IsInPvpProhibitedZone(areaId);
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
    return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedArea(uint32 id)
{
    return find(pvpProhibitedAreaIds.begin(), pvpProhibitedAreaIds.end(), id) != pvpProhibitedAreaIds.end();
}

bool PlayerbotAIConfig::IsRestrictedHealerDPSMap(uint32 mapId) const
{
    return restrictHealerDPS &&
            std::find(restrictedHealerDPSMaps.begin(), restrictedHealerDPSMaps.end(), mapId) != restrictedHealerDPSMaps.end();
}

std::string const PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour,
             aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(std::string const fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;

    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(std::make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen)  // close log file
        fclose(file);

    std::string m_logsDir = sConfigMgr->GetOption<std::string>("LogsDir", "", false);
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }

    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;

    return true;
}

void PlayerbotAIConfig::log(std::string const fileName, char const* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName) && !openLog(fileName, "a"))
        return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::loadWorldBuff()
{
    std::string matrix = sConfigMgr->GetOption<std::string>("AiPlayerbot.WorldBuffMatrix", "", true);
    if (matrix.empty())
        return;

    std::istringstream entryStream(matrix);
    std::string entry;

    while (std::getline(entryStream, entry, ';'))
    {

        entry.erase(0, entry.find_first_not_of(" \t\r\n"));
        entry.erase(entry.find_last_not_of(" \t\r\n") + 1);

        size_t firstColon = entry.find(':');
        size_t secondColon = entry.find(':', firstColon + 1);

        if (firstColon == std::string::npos || secondColon == std::string::npos)
        {
            LOG_ERROR("playerbots", "Malformed entry: [{}]", entry);
            continue;
        }

        std::string metaPart = entry.substr(firstColon + 1, secondColon - firstColon - 1);
        std::string spellPart = entry.substr(secondColon + 1);

        std::vector<uint32> ids;
        std::istringstream metaStream(metaPart);
        std::string token;
        while (std::getline(metaStream, token, ','))
        {
            try {
                ids.push_back(static_cast<uint32>(std::stoi(token)));
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid meta token in [{}]", entry);
                break;
            }
        }

        if (ids.size() != 5)
        {
            LOG_ERROR("playerbots", "Entry [{}] has incomplete meta block", entry);
            continue;
        }

        std::istringstream spellStream(spellPart);
        while (std::getline(spellStream, token, ','))
        {
            try {
                uint32 spellId = static_cast<uint32>(std::stoi(token));
                worldBuff wb = { spellId, ids[0], ids[1], ids[2], ids[3], ids[4] };
                worldBuffs.push_back(wb);
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid spell ID in [{}]", entry);
            }
        }
    }
}

static std::vector<std::string> split(const std::string& str, const std::string& pattern)
{
    std::vector<std::string> res;
    if (str == "")
        return res;
    // Also add separators to string connections to facilitate intercepting the last paragraph.
    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos)
    {
        std::string temp = strs.substr(0, pos);
        res.push_back(temp);
        // Remove the split string and split the remaining string
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

std::vector<std::vector<uint32>> PlayerbotAIConfig::ParseTempTalentsOrder(uint32 cls, std::string tab_link)
{
    // check bad link
    uint32 classMask = 1 << (cls - 1);
    std::vector<std::vector<uint32>> res;
    std::vector<std::string> tab_links = split(tab_link, "-");
    std::map<uint32, std::vector<TalentEntry const*>> spells;
    std::vector<std::vector<std::vector<uint32>>> orders(3);
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells[talentTabInfo->tabpage].push_back(talentInfo);
    }
    for (int tab = 0; tab < 3; tab++)
    {
        if (tab_links.size() <= tab)
        {
            break;
        }
        std::sort(spells[tab].begin(), spells[tab].end(),
                  [&](TalentEntry const* lhs, TalentEntry const* rhs)
                  { return lhs->Row != rhs->Row ? lhs->Row < rhs->Row : lhs->Col < rhs->Col; });
        for (int i = 0; i < tab_links[tab].size(); i++)
        {
            if (i >= spells[tab].size())
            {
                break;
            }
            int lvl = tab_links[tab][i] - '0';
            if (lvl == 0)
                continue;
            orders[tab].push_back({(uint32)tab, spells[tab][i]->Row, spells[tab][i]->Col, (uint32)lvl});
        }
    }
    // sort by talent tab size
    std::sort(orders.begin(), orders.end(), [&](auto& lhs, auto& rhs) { return lhs.size() > rhs.size(); });
    for (auto& order : orders)
    {
        res.insert(res.end(), order.begin(), order.end());
    }
    return res;
}

std::vector<std::vector<uint32>> PlayerbotAIConfig::ParseTempPetTalentsOrder(uint32 spec, std::string tab_link)
{
    // check bad link
    // uint32 classMask = 1 << (cls - 1);
    std::vector<TalentEntry const*> spells;
    std::vector<std::vector<uint32>> orders;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if (!((1 << spec) & talentTabInfo->petTalentMask))
            continue;
        // skip some duplicate spells like dash/dive
        if (talentInfo->TalentID == 2201 || talentInfo->TalentID == 2208 || talentInfo->TalentID == 2219 ||
            talentInfo->TalentID == 2203)
            continue;

        spells.push_back(talentInfo);
    }
    std::sort(spells.begin(), spells.end(),
              [&](TalentEntry const* lhs, TalentEntry const* rhs)
              { return lhs->Row != rhs->Row ? lhs->Row < rhs->Row : lhs->Col < rhs->Col; });
    for (int i = 0; i < tab_link.size(); i++)
    {
        if (i >= spells.size())
        {
            break;
        }
        int lvl = tab_link[i] - '0';
        if (lvl == 0)
            continue;
        orders.push_back({spells[i]->Row, spells[i]->Col, (uint32)lvl});
    }
    // sort by talent tab size
    std::sort(orders.begin(), orders.end(), [&](auto& lhs, auto& rhs) { return lhs.size() > rhs.size(); });

    return orders;
}
