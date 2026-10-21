/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Battleground.h"
#include "CellImpl.h"
#include "Creature.h"
#include "GameTime.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Item.h"
#include "LivingBondMgr.h"
#include "LivingWorldMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "WorldPacket.h"
#include "SpellInfo.h"

using LivingWorld::BondEvent;

namespace
{
LivingBondMgr& Bonds() { return LivingBondMgr::instance(); }

// Group members on the same map and close enough to have shared the moment.
std::vector<Player*> NearbyGroup(Player* player, float range)
{
    std::vector<Player*> members;
    Group* group = player->GetGroup();
    if (!group)
        return members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        if (Player* member = ref->GetSource())
            if (member != player && member->IsInWorld() && member->GetMapId() == player->GetMapId() &&
                member->IsWithinDistInMap(player, range))
                members.push_back(member);
    return members;
}

bool IsBodyArmour(ItemTemplate const* proto)
{
    if (proto->Class != ITEM_CLASS_ARMOR)
        return false;
    switch (proto->InventoryType)
    {
        case INVTYPE_HEAD:
        case INVTYPE_SHOULDERS:
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:
        case INVTYPE_WAIST:
        case INVTYPE_LEGS:
        case INVTYPE_FEET:
        case INVTYPE_WRISTS:
        case INVTYPE_HANDS:
            return true;
        default:
            return false;
    }
}

void RecordBossKill(Player* killer, Creature* killed)
{
    if (!killer || !killed || !(killed->IsDungeonBoss() || killed->isWorldBoss()) || !killer->GetMap()->IsDungeon())
        return;
    std::vector<Player*> party = NearbyGroup(killer, 200.0f);
    party.push_back(killer);
    for (Player* member : party)
        for (Player* other : party)
            if (member != other)
                Bonds().Record(member, other, BondEvent::DungeonBoss);
}

// Long self-contained buffs (Fortitude, Mark of the Wild, Blessings, Arcane Intellect...).
bool IsLongBuff(SpellInfo const* info)
{
    return info->IsPositive() && info->HasEffect(SPELL_EFFECT_APPLY_AURA) && info->GetMaxDuration() >= 10 * MINUTE * IN_MILLISECONDS;
}

bool IsHeal(SpellInfo const* info)
{
    return info->IsPositive() &&
           (info->HasEffect(SPELL_EFFECT_HEAL) || info->HasAura(SPELL_AURA_PERIODIC_HEAL));
}
}  // namespace

class LivingBondPlayerScript : public PlayerScript
{
public:
    LivingBondPlayerScript()
        : PlayerScript("LivingBondPlayerScript",
                       {PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST, PLAYERHOOK_ON_CREATURE_KILL,
                        PLAYERHOOK_ON_CREATURE_KILLED_BY_PET, PLAYERHOOK_ON_PVP_KILL,
                        PLAYERHOOK_ON_GROUP_ROLL_REWARD_ITEM, PLAYERHOOK_ON_DUEL_END, PLAYERHOOK_ON_TEXT_EMOTE,
                        PLAYERHOOK_ON_SPELL_CAST, PLAYERHOOK_CAN_SET_TRADE_ITEM})
    {
    }

    // Each member's own completion fires this, so a bot remembers the people it finished a quest with.
    void OnPlayerCompleteQuest(Player* player, Quest const* /*quest*/) override
    {
        for (Player* member : NearbyGroup(player, 100.0f))
            Bonds().Record(player, member, BondEvent::Quest);
    }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override { RecordBossKill(killer, killed); }
    void OnPlayerCreatureKilledByPet(Player* owner, Creature* killed) override { RecordBossKill(owner, killed); }

    // World PvP only: battleground kills are part of the match, not personal. Being killed again
    // soon after, or by someone far above your level, is a gank and stings more.
    void OnPlayerPVPKill(Player* killer, Player* killed) override
    {
        if (!killer || !killed || killer == killed || killed->InBattleground() || killed->InArena())
            return;
        uint32 seconds = 0;
        bool const killedBefore = Bonds().SecondsSince(killed->GetGUID().GetCounter(), killer->GetGUID().GetCounter(),
                                                        BondEvent::KilledBy, seconds);
        Bonds().Record(killed, killer, BondEvent::KilledBy);
        if (LivingWorld::IsGank(seconds, killedBefore, killer->GetLevel(), killed->GetLevel()))
            Bonds().Record(killed, killer, BondEvent::Ganked);
    }

    // Everyone who rolled for the item remembers a winner who needed gear it can't use or wouldn't wear.
    void OnPlayerGroupRollRewardItem(Player* winner, Item* item, uint32 /*count*/, RollVote voteType,
                                     Roll* roll) override
    {
        if (!winner || !item || !roll || voteType != NEED)
            return;
        ItemTemplate const* proto = item->GetTemplate();
        if (!proto || (proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON))
            return;
        bool const canUse = winner->CanUseItem(proto) == EQUIP_ERR_OK;
        bool const bodyArmour = IsBodyArmour(proto);
        uint8 const tier = bodyArmour ? static_cast<uint8>(proto->SubClass) : 0;
        if (!LivingWorld::IsNinjaNeed(canUse, bodyArmour, tier,
                                      LivingWorld::BestArmourTier(winner->getClass(), winner->GetLevel())))
            return;
        // One of the bots that wanted it says something (once: the others just remember).
        bool spoken = false;
        for (auto const& [guid, vote] : roll->playerVote)
            if (guid != winner->GetGUID() && (vote == NEED || vote == GREED))
                if (Player* rival = ObjectAccessor::FindPlayer(guid))
                    if (Bonds().Record(rival, winner, BondEvent::Ninja) && !spoken && rival->IsInWorld() &&
                        rival->IsWithinDistInMap(winner, 100.0f))
                    {
                        rival->Say(canUse ? "Really, " + winner->GetName() + "? That's not even your armour type."
                                          : "Really, " + winner->GetName() + "? You can't even use that.",
                                   LANG_UNIVERSAL);
                        spoken = true;
                    }
    }

    // A finished duel is sparring, win or lose (an interrupted one doesn't count).
    void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
    {
        if (!winner || !loser || type != DUEL_WON)
            return;
        Bonds().Record(winner, loser, BondEvent::Duel);
        Bonds().Record(loser, winner, BondEvent::Duel);
    }

    void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 /*emoteNum*/, ObjectGuid guid) override
    {
        int8 const tone = LivingWorld::EmoteTone(textEmote);
        if (!player || !tone || !guid.IsPlayer())
            return;
        if (Player* target = ObjectAccessor::FindPlayer(guid))
            if (target->IsWithinDistInMap(player, 40.0f))
                Bonds().Record(target, player, tone > 0 ? BondEvent::FriendlyEmote : BondEvent::RudeEmote);
    }

    // Help from outside the group stands out; inside a group, healing and buffing are expected.
    // Resurrection always counts.
    void OnPlayerSpellCast(Player* caster, Spell* spell, bool /*skipCheck*/) override
    {
        if (!caster || !spell)
            return;
        Unit* unitTarget = spell->m_targets.GetUnitTarget();
        Player* target = unitTarget ? unitTarget->ToPlayer() : nullptr;
        if (!target || target == caster)
            return;
        SpellInfo const* info = spell->GetSpellInfo();
        if (!info)
            return;
        if (info->HasEffect(SPELL_EFFECT_RESURRECT) || info->HasEffect(SPELL_EFFECT_RESURRECT_NEW))
        {
            Bonds().Record(target, caster, BondEvent::Resurrected);
            return;
        }
        if (caster->IsInSameGroupWith(target) || caster->IsHostileTo(target))
            return;
        if (IsHeal(info))
            Bonds().Record(target, caster, BondEvent::Healed);
        else if (IsLongBuff(info))
            Bonds().Record(target, caster, BondEvent::Buffed);
    }

    // The core has no trade-completed hook, so an item offered in the trade window counts as a gift
    // (its cooldown keeps the window from being used to farm friendship).
    bool OnPlayerCanSetTradeItem(Player* player, Item* /*tradedItem*/, uint8 /*tradeSlot*/) override
    {
        if (player)
            if (TradeData* trade = player->GetTradeData())
                Bonds().Record(trade->GetTrader(), player, BondEvent::Gift);
        return true;
    }
};

class LivingBondGroupScript : public GroupScript
{
public:
    LivingBondGroupScript()
        : GroupScript("LivingBondGroupScript", {GROUPHOOK_ON_ADD_MEMBER, GROUPHOOK_ON_INVITE_MEMBER,
                                                GROUPHOOK_ON_REMOVE_MEMBER, GROUPHOOK_ON_DISBAND})
    {
    }

    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        if (group->GetMembersCount() == 2)  // the leader was there first; start their clock too
            Bonds().OnGroupJoin(group->GetGUID().GetCounter(), group->GetLeaderGUID().GetCounter());
        Bonds().OnGroupJoin(group->GetGUID().GetCounter(), guid.GetCounter());
    }

    void OnInviteMember(Group* group, ObjectGuid guid) override
    {
        Player* invited = ObjectAccessor::FindPlayer(guid);
        Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
        Bonds().Record(invited, leader, BondEvent::Invited);
    }

    void OnRemoveMember(Group* group, ObjectGuid guid, RemoveMethod method, ObjectGuid kicker,
                        char const* /*reason*/) override
    {
        Player* removed = ObjectAccessor::FindPlayer(guid);
        if (!removed)
            return;
        if (method == GROUP_REMOVEMETHOD_KICK || method == GROUP_REMOVEMETHOD_KICK_LFG)
            Bonds().Record(removed, ObjectAccessor::FindPlayer(kicker), BondEvent::Kicked);
        else if (method == GROUP_REMOVEMETHOD_LEAVE && removed->GetMap() && removed->GetMap()->IsDungeon())
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                if (Player* member = ref->GetSource())
                    if (member != removed && member->GetMapId() == removed->GetMapId())
                        Bonds().Record(member, removed, BondEvent::Abandoned);
        Bonds().OnGroupLeave(group->GetGUID().GetCounter(), removed);
    }

    void OnDisband(Group* group) override { Bonds().OnGroupDisband(group->GetGUID().GetCounter()); }
};

class LivingBondBGScript : public BGScript
{
public:
    LivingBondBGScript() : BGScript("LivingBondBGScript") {}

    // Bots remember the real players who fought on their side (bot-to-bot pairs are skipped: a full
    // Alterac Valley would otherwise write thousands of rows).
    void OnBattlegroundEnd(Battleground* bg, TeamId winnerTeam) override
    {
        if (!bg || bg->isArena())
            return;
        std::vector<Player*> bots, players;
        for (auto const& [guid, player] : bg->GetPlayers())
            if (player && player->IsInWorld())
                (GET_PLAYERBOT_AI(player) ? bots : players).push_back(player);
        for (Player* bot : bots)
            for (Player* player : players)
                if (player->GetBgTeamId() == bot->GetBgTeamId())
                    Bonds().Record(bot, player,
                                   bot->GetBgTeamId() == winnerTeam ? BondEvent::BattlegroundWin
                                                                    : BondEvent::BattlegroundLoss);
    }
};

namespace
{
std::string LoginGreeting(LivingWorld::Topic topic, std::string const& name)
{
    switch (topic)
    {
        case LivingWorld::Topic::Dungeons: return "Hey " + name + "! Up for another dungeon run sometime?";
        case LivingWorld::Topic::Battlegrounds: return name + "! Good to see you. Queue some battlegrounds later?";
        case LivingWorld::Topic::Quests: return "Hey " + name + ", good to see you on. Need a hand with quests?";
        default: return "Hey " + name + "! Good to see you again.";
    }
}

void TextEmote(Player* bot, uint32 textEmote, Player* target)
{
    WorldPacket packet(CMSG_TEXT_EMOTE, 4 + 4 + 8);
    packet << textEmote << uint32(0) << target->GetGUID();
    bot->GetSession()->HandleTextEmoteOpcode(packet);
}

bool CanWhisper(Player* bot, Player* target)
{
    return bot && target && bot->IsInWorld() && target->IsInWorld() && bot->GetTeamId() == target->GetTeamId();
}
}  // namespace

// Friends greet: whispers when the player logs in (or when the bot comes online while the player is),
// a wave and a hello in person; rivals glare. Every greeting has a per-pair cooldown.
class LivingBondTalkScript : public PlayerScript
{
public:
    LivingBondTalkScript() : PlayerScript("LivingBondTalkScript", {PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_AFTER_UPDATE}) {}

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;
        // A bot's AI is attached after this hook, so the random-bot list (not IsRealPlayer) tells them apart.
        if (!sRandomPlayerbotMgr.IsRandomBot(player))
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _players[player->GetGUID().GetCounter()] = {0, uint32(GameTime::GetGameTime().count()) + LivingWorld::LoginGreetDelay};
            return;
        }
        // A friend bot starting its session says hello to its friends who are online (not during the mass
        // login after a restart).
        if (GameTime::GetUptime().count() < 15 * MINUTE || !LivingWorldMgr::instance().IsEnabled(player))
            return;
        for (auto const& [other, bond] : LivingBondMgr::instance().Strongest(player->GetGUID().GetCounter(), 5))
        {
            if (LivingWorld::AttitudeFor(bond.score) != LivingWorld::Attitude::Friend)
                continue;
            Player* friendPlayer = ObjectAccessor::FindPlayerByLowGUID(other);
            if (IsRealPlayer(friendPlayer) && CanWhisper(player, friendPlayer) &&
                LivingBondMgr::instance().TryGreet(player->GetGUID().GetCounter(), other, true))
                player->Whisper("Just logged on. Hi " + friendPlayer->GetName() + "!", LANG_UNIVERSAL, friendPlayer);
        }
    }

    void OnPlayerAfterUpdate(Player* player, uint32 diff) override
    {
        if (!player || !IsRealPlayer(player) || !player->IsInWorld())
            return;
        uint32 const guid = player->GetGUID().GetCounter();
        bool scan = false;
        bool loginGreet = false;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            State& state = _players[guid];
            state.scanTimer += diff;
            if (state.scanTimer >= 10 * IN_MILLISECONDS)
            {
                state.scanTimer = 0;
                scan = true;
            }
            if (state.loginGreetAt && GameTime::GetGameTime().count() >= state.loginGreetAt)
            {
                state.loginGreetAt = 0;
                loginGreet = true;
            }
        }
        if (loginGreet)
            GreetAtLogin(player);
        if (scan)
            GreetNearby(player);
    }

private:
    struct State
    {
        uint32 scanTimer = 0;
        uint32 loginGreetAt = 0;
    };

    static void GreetAtLogin(Player* player)
    {
        uint32 greeted = 0;
        for (auto const& [botGuid, bond] : LivingBondMgr::instance().KnownBy(player->GetGUID().GetCounter()))
        {
            if (greeted >= LivingWorld::MaxLoginGreetings || LivingWorld::AttitudeFor(bond.score) != LivingWorld::Attitude::Friend)
                break;
            Player* bot = ObjectAccessor::FindPlayerByLowGUID(botGuid);
            if (!CanWhisper(bot, player) || !LivingWorldMgr::instance().IsEnabled(bot) ||
                !LivingBondMgr::instance().TryGreet(botGuid, player->GetGUID().GetCounter(), true))
                continue;
            bot->Whisper(LoginGreeting(LivingWorld::SharedTopic(bond), player->GetName()), LANG_UNIVERSAL, player);
            ++greeted;
        }
    }

    static void GreetNearby(Player* player)
    {
        std::vector<Player*> nearby;
        Acore::AnyPlayerInObjectRangeCheck check(player, float(LivingWorld::GreetRange));
        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(player, nearby, check);
        Cell::VisitObjects(player, searcher, float(LivingWorld::GreetRange));
        uint32 const playerGuid = player->GetGUID().GetCounter();
        for (Player* bot : nearby)
        {
            if (bot == player || !LivingWorldMgr::instance().IsEnabled(bot) || bot->IsInCombat() || player->IsInCombat())
                continue;
            uint32 const botGuid = bot->GetGUID().GetCounter();
            LivingWorld::Attitude const attitude = LivingBondMgr::instance().AttitudeOf(botGuid, playerGuid);
            if (attitude == LivingWorld::Attitude::Neutral || attitude == LivingWorld::Attitude::Wary ||
                !LivingBondMgr::instance().TryGreet(botGuid, playerGuid, false))
                continue;
            switch (attitude)
            {
                case LivingWorld::Attitude::Friend:
                    TextEmote(bot, TEXT_EMOTE_WAVE, player);
                    bot->Say("Oh hey, " + player->GetName() + "!", LANG_UNIVERSAL);
                    // A friend without a guild gets offered this bot's, through the normal invite popup.
                    if (!player->GetGuildId() && player->GetTeamId() == bot->GetTeamId())
                        if (Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId()))
                            if (guild->HasRankRight(bot, GR_RIGHT_INVITE) &&
                                LivingBondMgr::instance().TryGuildInvite(botGuid, playerGuid))
                            {
                                bot->Say("You should join us in <" + guild->GetName() + ">!", LANG_UNIVERSAL);
                                guild->HandleInviteMember(bot->GetSession(), player->GetName());
                            }
                    break;
                case LivingWorld::Attitude::Friendly:
                    TextEmote(bot, TEXT_EMOTE_WAVE, player);
                    break;
                case LivingWorld::Attitude::Rival:
                    TextEmote(bot, TEXT_EMOTE_GLARE, player);
                    break;
                default:
                    break;
            }
        }
    }

    std::mutex _mutex;
    std::unordered_map<uint32, State> _players;
};

void AddLivingBondScripts()
{
    new LivingBondTalkScript();
    new LivingBondPlayerScript();
    new LivingBondGroupScript();
    new LivingBondBGScript();
}
