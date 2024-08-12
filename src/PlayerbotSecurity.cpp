/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotSecurity.h"

#include "LFGMgr.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

PlayerbotSecurity::PlayerbotSecurity(Player* const bot) : bot(bot)
{
    if (bot)
        account = sCharacterCache->GetCharacterAccountIdByGuid(bot->GetGUID());
}

PlayerbotSecurityLevel PlayerbotSecurity::LevelFor(Player* from, DenyReason* reason, bool ignoreGroup)
{
    if (from->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        return PLAYERBOT_SECURITY_ALLOW_ALL;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return PLAYERBOT_SECURITY_DENY_ALL;
    }
    if (botAI->IsOpposing(from))
    {
        if (reason)
            *reason = PLAYERBOT_DENY_OPPOSING;

        return PLAYERBOT_SECURITY_DENY_ALL;
    }

    if (sPlayerbotAIConfig->IsInRandomAccountList(account))
    {
        if (botAI->IsOpposing(from))
        {
            if (reason)
                *reason = PLAYERBOT_DENY_OPPOSING;

            return PLAYERBOT_SECURITY_DENY_ALL;
        }

        // if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
        // {
        //     if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
        //     {
        //         if (reason)
        //             *reason = PLAYERBOT_DENY_LFG;

        //         return PLAYERBOT_SECURITY_TALK;
        //     }
        // }

        Group* group = from->GetGroup();
        if (group && group == bot->GetGroup() && !ignoreGroup && botAI->GetMaster() == from)
        {
            return PLAYERBOT_SECURITY_ALLOW_ALL;
        }

        if (group && group == bot->GetGroup() && !ignoreGroup && botAI->GetMaster() != from)
        {
            if (reason)
                *reason = PLAYERBOT_DENY_NOT_YOURS;
            return PLAYERBOT_SECURITY_TALK;
        }

        if (sPlayerbotAIConfig->groupInvitationPermission <= 0)
        {
            if (reason)
                *reason = PLAYERBOT_DENY_NONE;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (sPlayerbotAIConfig->groupInvitationPermission <= 1 && (int32)bot->GetLevel() - (int8)from->GetLevel() > 5)
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_LOW_LEVEL;

                return PLAYERBOT_SECURITY_TALK;
            }
        }

        int32 botGS = (int32)botAI->GetEquipGearScore(bot, false, false);
        int32 fromGS = (int32)botAI->GetEquipGearScore(from, false, false);
        if (sPlayerbotAIConfig->gearscorecheck)
        {
            if (botGS && bot->GetLevel() > 15 && botGS > fromGS &&
                static_cast<float>(100 * (botGS - fromGS) / botGS) >=
                    static_cast<float>(12 * sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) / from->GetLevel()))
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_GEARSCORE;
                return PLAYERBOT_SECURITY_TALK;
            }
        }

        if (bot->InBattlegroundQueue())
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_BG;

                return PLAYERBOT_SECURITY_TALK;
            }
        }

        /*if (bot->isDead())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_DEAD;

            return PLAYERBOT_SECURITY_TALK;
        }*/

        group = bot->GetGroup();
        if (!group)
        {
            /*if (bot->GetMapId() != from->GetMapId() || bot->GetDistance(from) > sPlayerbotAIConfig->whisperDistance)
            {
                if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
                {
                    if (reason)
                        *reason = PLAYERBOT_DENY_FAR;

                    return PLAYERBOT_SECURITY_TALK;
                }
            }*/

            if (reason)
                *reason = PLAYERBOT_DENY_INVITE;

            return PLAYERBOT_SECURITY_INVITE;
        }

        if (group->IsFull())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_FULL_GROUP;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (group->GetLeaderGUID() != bot->GetGUID())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_NOT_LEADER;

            return PLAYERBOT_SECURITY_TALK;
        }
        else
        {
            if (reason)
                *reason = PLAYERBOT_DENY_IS_LEADER;

            return PLAYERBOT_SECURITY_INVITE;
        }

        if (reason)
            *reason = PLAYERBOT_DENY_INVITE;
        
        return PLAYERBOT_SECURITY_INVITE;
    }

    if (botAI->GetMaster() == from)
        return PLAYERBOT_SECURITY_ALLOW_ALL;

    if (reason)
        *reason = PLAYERBOT_DENY_NOT_YOURS;

    return PLAYERBOT_SECURITY_INVITE;
}

bool PlayerbotSecurity::CheckLevelFor(PlayerbotSecurityLevel level, bool silent, Player* from, bool ignoreGroup)
{
    DenyReason reason = PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel realLevel = LevelFor(from, &reason, ignoreGroup);
    if (realLevel >= level)
        return true;

    PlayerbotAI* fromBotAI = GET_PLAYERBOT_AI(from);
    if (silent || (fromBotAI && !fromBotAI->IsRealPlayer()))
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Player* master = botAI->GetMaster();
    if (master && botAI && botAI->IsOpposing(master) && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        return false;

    std::ostringstream out;
    switch (realLevel)
    {
        case PLAYERBOT_SECURITY_DENY_ALL:
            out << "I'm kind of busy now";
            break;
        case PLAYERBOT_SECURITY_TALK:
            switch (reason)
            {
                case PLAYERBOT_DENY_NONE:
                    out << "I'll do it later";
                    break;
                case PLAYERBOT_DENY_LOW_LEVEL:
                    out << "You are too low level: |cffff0000" << (uint32)from->GetLevel() << "|cffffffff/|cff00ff00"
                        << (uint32)bot->GetLevel();
                    break;
                case PLAYERBOT_DENY_GEARSCORE:
                {
                    int botGS = (int)botAI->GetEquipGearScore(bot, false, false);
                    int fromGS = (int)botAI->GetEquipGearScore(from, false, false);
                    int diff = (100 * (botGS - fromGS) / botGS);
                    int req = 12 * sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) / from->GetLevel();
                    out << "Your gearscore is too low: |cffff0000" << fromGS << "|cffffffff/|cff00ff00" << botGS
                        << " |cffff0000" << diff << "%|cffffffff/|cff00ff00" << req << "%";
                }
                break;
                case PLAYERBOT_DENY_NOT_YOURS:
                    out << "I have a master already";
                    break;
                case PLAYERBOT_DENY_IS_BOT:
                    out << "You are a bot";
                    break;
                case PLAYERBOT_DENY_OPPOSING:
                    out << "You are the enemy";
                    break;
                case PLAYERBOT_DENY_DEAD:
                    out << "I'm dead. Will do it later";
                    break;
                case PLAYERBOT_DENY_INVITE:
                    out << "Invite me to your group first";
                    break;
                case PLAYERBOT_DENY_FAR:
                {
                    out << "You must be closer to invite me to your group. I am in ";

                    if (AreaTableEntry const* entry = sAreaTableStore.LookupEntry(bot->GetAreaId()))
                    {
                        out << " |cffffffff(|cffff0000" << entry->area_name[0] << "|cffffffff)";
                    }
                }
                break;
                case PLAYERBOT_DENY_FULL_GROUP:
                    out << "I am in a full group. Will do it later";
                    break;
                case PLAYERBOT_DENY_IS_LEADER:
                    out << "I am currently leading a group. I can invite you if you want.";
                    break;
                case PLAYERBOT_DENY_NOT_LEADER:
                    if (botAI->GetGroupMaster())
                    {
                        out << "I am in a group with " << botAI->GetGroupMaster()->GetName()
                            << ". You can ask him for invite.";
                    }
                    else
                    {
                        out << "I am in a group with someone else. You can ask him for invite.";
                    }
                    break;
                case PLAYERBOT_DENY_BG:
                    out << "I am in a queue for BG. Will do it later";
                    break;
                case PLAYERBOT_DENY_LFG:
                    out << "I am in a queue for dungeon. Will do it later";
                    break;
                default:
                    out << "I can't do that";
                    break;
            }
            break;
        case PLAYERBOT_SECURITY_INVITE:
            out << "Invite me to your group first";
            break;
        default:
            out << "I can't do that";
            break;
    }

    std::string const text = out.str();
    ObjectGuid guid = from->GetGUID();
    time_t lastSaid = whispers[guid][text];
    if (!lastSaid || (time(nullptr) - lastSaid) >= sPlayerbotAIConfig->repeatDelay / 1000)
    {
        whispers[guid][text] = time(nullptr);
        bot->Whisper(text, LANG_UNIVERSAL, from);
    }

    return false;
}
