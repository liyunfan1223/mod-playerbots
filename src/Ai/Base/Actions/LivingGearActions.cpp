/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingGearActions.h"

#include "DBCStores.h"
#include "LivingGearPolicy.h"
#include "LivingWorldMgr.h"
#include "Playerbots.h"
#include "TravelMgr.h"

namespace
{
// Reuses the bot's own equip evaluation (stats weights, proficiency, uniqueness) without
// registering a cached "item usage" value per candidate item.
class GearUpgradeCheck : public ItemUsageValue
{
public:
    explicit GearUpgradeCheck(PlayerbotAI* ai) : ItemUsageValue(ai, "living gear upgrade") {}
    bool IsUpgrade(ItemTemplate const* item)
    {
        ItemUsage const usage = QueryItemUsageForEquip(item);
        return usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE;
    }
};

bool NearHuntingGround(Player* bot, std::vector<WorldLocation> const& locs)
{
    for (WorldLocation const& loc : locs)
        if (loc.GetMapId() == bot->GetMapId() && bot->GetExactDist(loc) <= LivingWorld::GearHuntDistance)
            return true;
    return false;
}
}  // namespace

LivingGearHunt LivingGearHuntValue::Calculate()
{
    LivingGearHunt hunt;
    if (!LivingWorldMgr::instance().IsEnabled(bot) ||
        LivingWorldMgr::instance().GetProfile(bot).motivation != LivingWorld::Motivation::Gear ||
        bot->GetMap()->IsDungeon() || bot->InBattleground())
        return hunt;

    GearUpgradeCheck check(botAI);
    std::vector<std::pair<uint32, uint32>> scored;  // creature, upgrades
    uint32 considered = 0;
    for (uint32 entry : sTravelMgr.GetGearHuntEntries(static_cast<uint8>(bot->GetLevel())))
    {
        auto const* locs = sTravelMgr.GetGearHuntLocs(entry);
        auto const* drops = sTravelMgr.GetGearDrops(entry);
        if (!locs || !drops || !NearHuntingGround(bot, *locs))
            continue;
        if (++considered > LivingWorld::MaxGearCandidates)
            break;

        uint32 upgrades = 0;
        for (uint32 itemId : *drops)
        {
            ItemTemplate const* item = sObjectMgr->GetItemTemplate(itemId);
            if (item && LivingWorld::CanWearSoon(item->RequiredLevel, bot->GetLevel()) && check.IsUpgrade(item))
                ++upgrades;
        }
        if (upgrades)
            scored.emplace_back(entry, upgrades);
    }

    std::sort(scored.begin(), scored.end(), [](auto const& a, auto const& b) { return a.second > b.second; });
    for (auto const& [entry, upgrades] : scored)
    {
        if (hunt.creatures.size() >= LivingWorld::MaxGearTargets)
            break;
        hunt.creatures.push_back(entry);
        hunt.upgrades += upgrades;
    }
    return hunt;
}

namespace LivingGear
{
bool SelectHuntPos(Player* bot, WorldPosition& out)
{
    PlayerbotAI* ai = GET_PLAYERBOT_AI(bot);
    if (!ai || !LivingWorldMgr::instance().IsEnabled(bot))
        return false;

    LivingGearHunt const hunt = ai->GetAiObjectContext()->GetValue<LivingGearHunt>("living gear hunt")->Get();
    std::vector<WorldLocation> grounds;
    for (uint32 entry : hunt.creatures)
        if (auto const* locs = sTravelMgr.GetGearHuntLocs(entry))
            for (WorldLocation const& loc : *locs)
                if (loc.GetMapId() == bot->GetMapId() && bot->GetExactDist(loc) <= LivingWorld::GearHuntDistance)
                    grounds.push_back(loc);
    if (grounds.empty())
        return false;

    out = grounds[urand(0, grounds.size() - 1)];
    LOG_DEBUG("playerbots", "[Living gear] {} hunts {} wanted drop(s) at map {} ({:.0f}, {:.0f})", bot->GetName(),
              hunt.upgrades, out.GetMapId(), out.GetPositionX(), out.GetPositionY());
    return true;
}

void FilterDungeons(PlayerbotAI* ai, std::set<uint32>& dungeons)
{
    Player* bot = ai->GetBot();
    if (!LivingWorldMgr::instance().IsEnabled(bot) ||
        LivingWorldMgr::instance().GetProfile(bot).motivation != LivingWorld::Motivation::Gear)
        return;

    GearUpgradeCheck check(ai);
    std::set<uint32> wanted;
    for (uint32 dungeonId : dungeons)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
        auto const* drops = dungeon ? sTravelMgr.GetDungeonGearDrops(dungeon->MapID) : nullptr;
        if (!drops)
            continue;
        for (uint32 itemId : *drops)
        {
            ItemTemplate const* item = sObjectMgr->GetItemTemplate(itemId);
            if (item && LivingWorld::CanWearSoon(item->RequiredLevel, bot->GetLevel()) && check.IsUpgrade(item))
            {
                wanted.insert(dungeonId);
                break;
            }
        }
    }
    if (!wanted.empty())
    {
        LOG_DEBUG("playerbots", "[Living gear] {} queues for {} of {} dungeons with wanted boss loot",
                  bot->GetName(), wanted.size(), dungeons.size());
        dungeons = std::move(wanted);
    }
}
}  // namespace LivingGear
