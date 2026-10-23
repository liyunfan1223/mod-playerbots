/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TrainerAction.h"
#include "AiFactory.h"
#include "BisListMgr.h"
#include "BudgetValues.h"
#include "Event.h"
#include "PlayerbotFactory.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "ReputationMgr.h"
#include "Trainer.h"

bool TrainerAction::Execute(Event event)
{
    std::string const param = event.getParam();

    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(target->GetEntry());
    if (!trainer)
        return false;

    // NOTE: Original version uses SpellIds here, but occasionally only inserts
    // a single spell ID value from parameters. If someone wants to impl multiple
    // spells as parameters, check SkipSpellsListAction::parseIds as an example.
    uint32 spellId = chat->parseSpell(param);

    bool learnSpells = param.find("learn") != std::string::npos || sRandomPlayerbotMgr.IsRandomBot(bot) ||
                       (sPlayerbotAIConfig.AllowLearnTrainerSpells &&
                        // TODO: Rewrite to only exclude start primary profession skills and make config dependent.
                        (trainer->GetTrainerType() != Trainer::Type::Tradeskill || !IsRealPlayer(botAI->GetMaster())));

    Iterate(target, learnSpells, spellId);

    return true;
}

bool TrainerAction::isUseful()
{
    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld() || !target->IsAlive())
        return false;

    return target->IsTrainer();
}

bool TrainerAction::isPossible()
{
    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(target->GetEntry());
    if (!trainer)
        return false;

    if (!trainer->IsTrainerValidForPlayer(bot))
        return false;

    for (Trainer::Spell const& spell : trainer->GetSpells())
    {
        Trainer::Spell const* trainerSpell = trainer->GetSpell(spell.SpellId);
        if (!trainerSpell)
            continue;

        if (!PlayerbotFactory::IsTrainerSpellAllowedForBot(bot, trainer, trainerSpell))
            continue;

        if (trainer->CanTeachSpell(bot, trainerSpell))
            return true;
    }

    return false;
}

Unit* TrainerAction::GetTarget()
{
    // There are just two scenarios: the bot has a master or it doesn't. If the
    // bot has a master, the master should target a unit; otherwise, the bot
    // should target the unit itself.
    if (Player* master = GetMaster())
        return master->GetSelectedUnit();

    return bot->GetSelectedUnit();
}

Creature* TrainerAction::GetCreatureTarget()
{
    Unit* target = GetTarget();
    return target ? target->ToCreature() : nullptr;
}

void TrainerAction::Iterate(Creature* creature, bool learnSpells, uint32 spellId)
{
    TellHeader(creature);

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(creature->GetEntry());
    if (!trainer)
        return;

    float reputationDiscount = bot->GetReputationPriceDiscount(creature);
    uint32 totalCost = 0;

    for (auto& spell : trainer->GetSpells())
    {
        // simplified version of Trainer::TeachSpell method

        Trainer::Spell const* trainerSpell = trainer->GetSpell(spell.SpellId);
        if (!trainerSpell)
            continue;

        if (!PlayerbotFactory::IsTrainerSpellAllowedForBot(bot, trainer, trainerSpell))
            continue;

        if (!trainer->CanTeachSpell(bot, trainerSpell))
            continue;

        if (spellId && trainerSpell->SpellId != spellId)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(trainerSpell->SpellId);
        if (!spellInfo)
            continue;

        uint32 cost = static_cast<uint32>(floor(trainerSpell->MoneyCost * reputationDiscount));
        totalCost += cost;

        std::ostringstream out;
        out << chat->FormatSpell(spellInfo) << chat->formatMoney(cost);

        if (learnSpells)
            Learn(spellInfo, cost, out);

        botAI->TellMaster(out);
    }

    TellFooter(totalCost);
}

void TrainerAction::Learn(SpellInfo const* spellInfo, uint32 cost, std::ostringstream& out)
{
    if (!botAI->HasCheat(BotCheatMask::gold))
    {
        if (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells) < cost)
        {
            out << " - too expensive";
            return;
        }

        bot->ModifyMoney(-static_cast<int32>(cost));
    }

    if (spellInfo->HasEffect(SPELL_EFFECT_LEARN_SPELL))
        bot->CastSpell(bot, spellInfo->Id, true);
    else
        bot->learnSpell(spellInfo->Id, false);

    out << " - learned";
}

void TrainerAction::TellHeader(Creature* creature)
{
    std::ostringstream out;
    out << "--- Can learn from " << creature->GetName() << " ---";
    botAI->TellMaster(out);
}

void TrainerAction::TellFooter(uint32 totalCost)
{
    if (totalCost)
    {
        std::ostringstream out;
        out << "Total cost: " << chat->formatMoney(totalCost);
        botAI->TellMaster(out);
    }
}

bool MaintenanceAction::Execute(Event /*event*/)
{
    if (!sPlayerbotAIConfig.MaintenanceCommand)
    {
        botAI->TellError("maintenance command is not allowed, please check the configuration.");
        return false;
    }

    botAI->TellMaster("I'm maintaining");
    PlayerbotFactory factory(bot, bot->GetLevel());

    if (!botAI->IsAltBot())
    {
        factory.InitAttunementQuests();
        factory.InitBags(false);
        factory.InitAmmo();
        factory.InitFood();
        factory.InitReagents();
        factory.InitConsumables();
        factory.InitPotions();
        factory.InitTalentsTree(true);
        factory.InitPet();
        factory.InitPetTalents();
        factory.InitSkills();
        factory.InitClassSpells();
        factory.InitAvailableSpells();
        factory.InitReputation();
        factory.InitSpecialSpells();
        factory.InitMounts();
        factory.InitGlyphs(false);
        factory.InitKeyring();
        if (bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
            factory.ApplyEnchantAndGemsNew();
    }
    else
    {
        if (sPlayerbotAIConfig.AltMaintenanceAttunementQs)
            factory.InitAttunementQuests();

        if (sPlayerbotAIConfig.AltMaintenanceBags)
            factory.InitBags(false);

        if (sPlayerbotAIConfig.AltMaintenanceAmmo)
            factory.InitAmmo();

        if (sPlayerbotAIConfig.AltMaintenanceFood)
            factory.InitFood();

        if (sPlayerbotAIConfig.AltMaintenanceReagents)
            factory.InitReagents();

        if (sPlayerbotAIConfig.AltMaintenanceConsumables)
            factory.InitConsumables();

        if (sPlayerbotAIConfig.AltMaintenancePotions)
            factory.InitPotions();

        if (sPlayerbotAIConfig.AltMaintenanceTalentTree)
            factory.InitTalentsTree(true);

        if (sPlayerbotAIConfig.AltMaintenancePet)
            factory.InitPet();

        if (sPlayerbotAIConfig.AltMaintenancePetTalents)
            factory.InitPetTalents();

        if (sPlayerbotAIConfig.AltMaintenanceSkills)
            factory.InitSkills();

        if (sPlayerbotAIConfig.AltMaintenanceClassSpells)
            factory.InitClassSpells();

        if (sPlayerbotAIConfig.AltMaintenanceAvailableSpells)
            factory.InitAvailableSpells();

        if (sPlayerbotAIConfig.AltMaintenanceReputation)
            factory.InitReputation();

        if (sPlayerbotAIConfig.AltMaintenanceSpecialSpells)
            factory.InitSpecialSpells();

        if (sPlayerbotAIConfig.AltMaintenanceMounts)
            factory.InitMounts();

        if (sPlayerbotAIConfig.AltMaintenanceGlyphs)
            factory.InitGlyphs(false);

        if (sPlayerbotAIConfig.AltMaintenanceKeyring)
            factory.InitKeyring();

        if (sPlayerbotAIConfig.AltMaintenanceGemsEnchants &&
            bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
            factory.ApplyEnchantAndGemsNew();
    }

    bot->DurabilityRepairAll(false, 1.0f, false);
    bot->SendTalentsInfoData(false);

    return true;
}

bool BisGearAction::RunAutogearFallback(uint16 effectiveIlvl)
{
    if (!sPlayerbotAIConfig.AutoGearCommand)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_autogear_unavailable_error",
            "autogear command is not allowed, please check the configuration.", {}));
        return false;
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "bis_no_rows_fallback",
        "No BiS for your tier/spec/level, check cfg, running autogear instead", {}));

    // Wipe all equipped slots so autogear gears from scratch at the requested ilvl
    // (avoids old high-tier items surviving the incremental 1.2x threshold).
    PlayerbotFactory::DestroyEquippedGear(bot);

    PlayerbotFactory::AutoGear(bot, sPlayerbotAIConfig.AutoGearQualityLimit, effectiveIlvl, /*incremental*/ false,
                               sPlayerbotAIConfig.TwoRoundsGearInit);
    return true;
}

bool BisGearAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig.AutoGearBisCommand)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_command_unavailable_error",
            "bis command is not allowed, please check the configuration.", {}));
        return false;
    }

    if (!sPlayerbotAIConfig.AutoGearCommandAltBots &&
        !sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_altbot_refused_error", "You cannot use bis on alt bots.", {}));
        return false;
    }

    if (sPlayerbotAIConfig.AutoGearQualityLimit < 4)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_quality_floor_error", "AutoGearQualityLimit must be 4 for BiS.", {}));
        return false;
    }

    if (sRandomPlayerbotMgr.IsSpecPvp(bot->GetGUID().GetCounter(), bot->getClass()))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_pvp_refused_error", "bis is PvE only, bot is configured as PvP.", {}));
        return false;
    }

    uint16 ilvl = static_cast<uint16>(sPlayerbotAIConfig.AutoGearScoreLimit);

    // Optional explicit ilvl override: `/p autogear bis 55`.
    // Garbage or out-of-range args are hard-rejected: no autogear fallback, no gear change.
    std::string const param = event.getParam();
    if (!param.empty())
    {
        unsigned long parsed = 0;
        size_t pos = 0;
        bool valid = false;
        try
        {
            parsed = std::stoul(param, &pos);
            valid = (parsed > 0 && pos == param.size() && parsed <= 0xFFFFu);
        }
        catch (...)
        {
            valid = false;
        }

        if (!valid)
        {
            std::map<std::string, std::string> phs;
            phs["%param"] = param;
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "bis_invalid_arg_error",
                "Invalid BiS ilvl argument '%param'. Use a positive integer.", phs));
            return false;
        }
        if (parsed > static_cast<unsigned long>(sPlayerbotAIConfig.AutoGearScoreLimit))
        {
            std::map<std::string, std::string> phs;
            phs["%requested"] = std::to_string(parsed);
            phs["%limit"] = std::to_string(sPlayerbotAIConfig.AutoGearScoreLimit);
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "bis_arg_above_limit_error",
                "BiS ilvl %requested exceeds AutoGearScoreLimit %limit, refusing", phs));
            return false;
        }
        ilvl = static_cast<uint16>(parsed);
    }
    uint8 cls = bot->getClass();
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    uint8 faction = bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 2;

    // Druid Bear (Feral Tank) shares tab 1 with Cat. Use sentinel tab 10 when tank strategy active.
    constexpr uint8 BIS_TAB_DRUID_BEAR = 10;
    constexpr uint16 BIS_ILVL_FALLBACK_WINDOW = 20;
    uint16 resolvedIlvl = 0;
    std::map<uint8, uint32> bisMap;
    if (cls == CLASS_DRUID && tab == DRUID_TAB_FERAL && PlayerbotAI::IsTank(bot))
        bisMap = sBisListMgr->GetBisForNearest(ilvl, BIS_ILVL_FALLBACK_WINDOW, cls, BIS_TAB_DRUID_BEAR, faction,
                                               &resolvedIlvl);
    if (bisMap.empty())
        bisMap = sBisListMgr->GetBisForNearest(ilvl, BIS_ILVL_FALLBACK_WINDOW, cls, tab, faction, &resolvedIlvl);

    // No rows within fallback window -> full autogear fallback at the effective ilvl.
    if (bisMap.empty())
    {
        std::map<std::string, std::string> phs;
        phs["%ilvl"] = std::to_string(ilvl);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_no_rows_autogear_msg",
            "No BiS at ilvl %ilvl, using Autogear %ilvl instead", phs));
        return RunAutogearFallback(ilvl);
    }

    if (resolvedIlvl != ilvl)
    {
        std::map<std::string, std::string> phs;
        phs["%requested"] = std::to_string(ilvl);
        phs["%resolved"] = std::to_string(resolvedIlvl);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_closest_match_msg",
            "No BiS at ilvl %requested, using closest match at ilvl %resolved", phs));
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "bis_applying_msg", "Applying BiS gear", {}));

    // 1. Wipe everything currently equipped so autogear starts from a clean slate.
    //    Old items linger in inventory otherwise and autogear leaves slots empty on bag conflicts.
    PlayerbotFactory::DestroyEquippedGear(bot);

    // Wipe equippable items from bags too. Autogear can shove old equipped items into bags
    // (HandleAutoStoreBagItemOpcode), and a unique-equipped duplicate stuck in a bag blocks
    // CanEquipNewItem on subsequent BiS runs. Spare consumables/reagents.
    auto destroyIfEquippable = [&](uint8 bag, uint8 slot)
    {
        Item* item = bot->GetItemByPos(bag, slot);
        if (!item)
            return;
        ItemTemplate const* tmpl = item->GetTemplate();
        if (!tmpl)
            return;
        if (tmpl->Class == ITEM_CLASS_WEAPON || tmpl->Class == ITEM_CLASS_ARMOR)
            bot->DestroyItem(bag, slot, true);
    };
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        destroyIfEquippable(INVENTORY_SLOT_BAG_0, slot);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag* container = bot->GetBagByPos(bag))
            for (uint32 slot = 0; slot < container->GetBagSize(); ++slot)
                destroyIfEquippable(bag, slot);
    }

    // 2. Run full autogear on the empty bot so every slot gets a best-available pick.
    //    Uncovered slots will keep the autogear pick; BiS overwrites the rest below.
    if (sPlayerbotAIConfig.AutoGearCommand)
        PlayerbotFactory::AutoGear(bot, sPlayerbotAIConfig.AutoGearQualityLimit, ilvl, /*incremental*/ false,
                                   sPlayerbotAIConfig.TwoRoundsGearInit, /*applyFinishers*/ false);

    // Autogear may have placed the exact item BiS wants into trinket2/finger2 (or vice versa);
    // unique-equipped enforcement would then make BiS's equip silently drop one copy.
    std::set<uint32> bisEntries;
    for (auto const& kv : bisMap)
        bisEntries.insert(kv.second);
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            if (bisEntries.count(item->GetEntry()))
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
    }

    // 3. Apply BiS: only touch slots where the bot can actually equip the BiS item.
    //    If item requires reputation, grant the required rank first. If CanUseItem still
    //    fails (class/race/skill/level), keep autogear's pick for that slot.
    for (auto const& kv : bisMap)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(kv.second);
        if (!proto)
            continue;

        // Grant required reputation rank if the item gates on it.
        if (proto->RequiredReputationFaction && proto->RequiredReputationRank > 0)
        {
            if (FactionEntry const* fac = sFactionStore.LookupEntry(proto->RequiredReputationFaction))
            {
                ReputationRank requiredRank = static_cast<ReputationRank>(proto->RequiredReputationRank);
                if (bot->GetReputationRank(proto->RequiredReputationFaction) < requiredRank)
                {
                    int32 standing = ReputationMgr::ReputationRankToStanding(
                                         static_cast<ReputationRank>(requiredRank - 1)) + 1;
                    bot->GetReputationMgr().SetReputation(fac, standing);
                }
            }
        }

        if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        uint8 slot = kv.first;
        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

        uint16 dest = 0;
        InventoryResult eqResult = bot->CanEquipNewItem(slot, dest, kv.second, false);

        // Paired slots (finger 10<->11, trinket 12<->13): destroy paired slot and retry once
        // when unique-equipped or autogear residue blocks the first attempt.
        if (eqResult != EQUIP_ERR_OK)
        {
            uint8 pairedSlot = 0xFF;
            if (slot == EQUIPMENT_SLOT_FINGER1)        pairedSlot = EQUIPMENT_SLOT_FINGER2;
            else if (slot == EQUIPMENT_SLOT_FINGER2)   pairedSlot = EQUIPMENT_SLOT_FINGER1;
            else if (slot == EQUIPMENT_SLOT_TRINKET1)  pairedSlot = EQUIPMENT_SLOT_TRINKET2;
            else if (slot == EQUIPMENT_SLOT_TRINKET2)  pairedSlot = EQUIPMENT_SLOT_TRINKET1;

            if (pairedSlot != 0xFF)
            {
                if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, pairedSlot))
                    bot->DestroyItem(INVENTORY_SLOT_BAG_0, pairedSlot, true);
                eqResult = bot->CanEquipNewItem(slot, dest, kv.second, false);
            }
        }

        if (eqResult == EQUIP_ERR_OK)
        {
            bot->EquipNewItem(dest, kv.second, true);
            bot->AutoUnequipOffhandIfNeed();
        }
    }

    PlayerbotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_EPIC, 0);
    factory.InitAmmo();
    if (bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
        factory.ApplyEnchantAndGemsNew();

    bot->DurabilityRepairAll(false, 1.0f, false);

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "bis_applied_msg", "BiS applied", {}));
    return true;
}

bool RemoveGlyphAction::Execute(Event /*event*/)
{
    for (uint32 slotIndex = 0; slotIndex < MAX_GLYPH_SLOT_INDEX; ++slotIndex)
    {
        bot->SetGlyph(slotIndex, 0, true);
    }
    bot->SendTalentsInfoData(false);
    return true;
}

static uint32 GetWornItemLevelAverage(Player* player)
{
    uint32 sumLevel = 0;
    uint32 count = 0;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            sumLevel += item->GetTemplate()->ItemLevel;
            ++count;
        }
    }

    return count ? sumLevel / count : 0;
}

bool AutoGearAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig.AutoGearCommand)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "autogear_command_disabled_error", "autogear command is not allowed, please check the configuration.", {}));
        return false;
    }

    if (!sPlayerbotAIConfig.AutoGearCommandAltBots &&
        !sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault("autogear_altbot_refused_error",
                                                                          "You cannot use autogear on alt bots.", {}));
        return false;
    }

    uint32 const qualityCap = static_cast<uint32>(sPlayerbotAIConfig.AutoGearQualityLimit);
    uint32 const ilvlCap = static_cast<uint32>(sPlayerbotAIConfig.AutoGearScoreLimit);  // 0 == no limit

    uint32 quality = qualityCap;
    uint32 ilvl = ilvlCap;

    // get optional chat quality
    std::string param = event.getParam();
    std::transform(param.begin(), param.end(), param.begin(), [](unsigned char c) { return std::tolower(c); });
    size_t const first = param.find_first_not_of(" \t");
    if (first == std::string::npos)
        param.clear();
    else
        param = param.substr(first, param.find_last_not_of(" \t") - first + 1);

    // 'reset' erases all equipped gear and regears from scratch instead of upgrading incrementally.
    // It can be combined with a quality/ilvl argument: 'autogear reset green', 'autogear reset 200'.
    bool reset = false;
    if (param == "reset" || param.rfind("reset ", 0) == 0)
    {
        reset = true;
        param = param.size() > 5 ? param.substr(6) : "";
        size_t const start = param.find_first_not_of(" \t");
        param = start == std::string::npos ? "" : param.substr(start);
    }

    if (!param.empty())
    {
        if (param == "match")
        {
            Player* master = GetMaster();
            if (!master)
            {
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_no_master_error", "I have no master to match gear with.", {}));
                return false;
            }

            uint32 const requestedIlvl = GetWornItemLevelAverage(master);
            if (requestedIlvl == 0)
            {
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_master_no_gear_error", "You have no gear equipped for me to match.", {}));
                return false;
            }

            ilvl = (ilvlCap == 0) ? requestedIlvl : std::min(requestedIlvl, ilvlCap);
            if (ilvlCap != 0 && requestedIlvl > ilvlCap)
            {
                std::map<std::string, std::string> phs;
                phs["%limit"] = std::to_string(ilvlCap);
                phs["%requested"] = std::to_string(requestedIlvl);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_ilvl_capped_msg",
                    "Config caps item level at %limit, gearing to that instead of %requested.", phs));
            }
        }
        else if (ChatHelper::parseItemQuality(param) != MAX_ITEM_QUALITY)
        {

            uint32 const requestedQuality = std::max<uint32>(ChatHelper::parseItemQuality(param), ITEM_QUALITY_NORMAL);

            quality = std::min(requestedQuality, qualityCap);
            if (requestedQuality > qualityCap)
            {
                std::map<std::string, std::string> phs;
                phs["%quality"] = ChatHelper::FormatItemQuality(qualityCap);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_quality_capped_msg", "Config limits me to %quality quality, gearing to that.", phs));
            }
        }
        else
        {
            // Handle negative numbers, garbage, and numbers that are too large to fit in a uint32.
            constexpr size_t MAX_ILVL_DIGITS = 5;
            bool const digitsOnly = !param.empty() && param.size() <= MAX_ILVL_DIGITS &&
                                    param.find_first_not_of("0123456789") == std::string::npos;
            uint32 const requestedIlvl = digitsOnly ? static_cast<uint32>(std::stoul(param)) : 0;

            if (!requestedIlvl)
            {
                std::map<std::string, std::string> phs;
                phs["%param"] = param;
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_invalid_arg_error",
                    "Unknown autogear option '%param'. Use bis, reset, match, a quality "
                    "(white, green, blue, purple, orange) or a positive item level number.",
                    phs));
                return false;
            }

            if (requestedIlvl <= ITEM_QUALITY_LEGENDARY)
            {
                std::map<std::string, std::string> phs;
                phs["%param"] = param;
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_quality_as_number_error",
                    "'%param' looks like a quality. Numbers are item levels (e.g. 200); use a colour word "
                    "for quality: white, green, blue, purple, orange.",
                    phs));
                return false;
            }

            ilvl = (ilvlCap == 0) ? requestedIlvl : std::min(requestedIlvl, ilvlCap);
            if (ilvlCap != 0 && requestedIlvl > ilvlCap)
            {
                std::map<std::string, std::string> phs;
                phs["%limit"] = std::to_string(ilvlCap);
                phs["%requested"] = std::to_string(requestedIlvl);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_ilvl_capped_msg",
                    "Config caps item level at %limit, gearing to that instead of %requested.", phs));
            }
        }
    }

    std::map<std::string, std::string> announcePhs;
    announcePhs["%quality"] = ChatHelper::FormatItemQuality(quality);
    announcePhs["%ilvl"] = std::to_string(ilvl);
    if (reset)
        botAI->TellMaster(
            ilvl != 0
                ? PlayerbotTextMgr::instance().GetBotTextOrDefault(
                      "autogear_reset_start_msg",
                      "I'm erasing my gear and regearing from scratch (%quality, up to item level %ilvl).", announcePhs)
                : PlayerbotTextMgr::instance().GetBotTextOrDefault(
                      "autogear_reset_start_any_ilvl_msg", "I'm erasing my gear and regearing from scratch (%quality).",
                      announcePhs));
    else
        botAI->TellMaster(
            ilvl != 0 ? PlayerbotTextMgr::instance().GetBotTextOrDefault(
                            "autogear_start_msg", "I'm auto gearing (%quality, up to item level %ilvl).", announcePhs)
                      : PlayerbotTextMgr::instance().GetBotTextOrDefault("autogear_start_any_ilvl_msg",
                                                                         "I'm auto gearing (%quality).", announcePhs));

    if (reset)
    {
        // Wipe everything equipped first so old high-tier items can't survive; then gear every
        // slot from scratch instead of only upgrading past the incremental threshold.
        PlayerbotFactory::DestroyEquippedGear(bot);
        PlayerbotFactory::AutoGear(bot, quality, ilvl, /*incremental*/ false, sPlayerbotAIConfig.TwoRoundsGearInit);
    }
    else
        PlayerbotFactory::AutoGear(bot, quality, ilvl, /*incremental*/ true);

    return true;
}
