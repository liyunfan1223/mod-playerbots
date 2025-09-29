#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AiObjectContext.h"
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Position.h"

namespace 
{
    // Big Bad Wolf
    static int currentIndex = 0;
    // Netherspite
    static std::map<ObjectGuid, uint32> beamMoveTimes;
    static std::map<ObjectGuid, bool> lastBeamMoveSideways;
}

bool KarazhanAttumenTheHuntsmanStackBehindAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* boss = karazhanHelper.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    float distance = 5.0f;
    float orientation = boss->GetOrientation() + M_PI;
    float x = boss->GetPositionX();
    float y = boss->GetPositionY();
    float z = boss->GetPositionZ();
    float rx = x + cos(orientation) * distance;
    float ry = y + sin(orientation) * distance;

    if (bot->GetExactDist2d(rx, ry) > 1.0f)
    {
        return MoveTo(bot->GetMapId(), rx, ry, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool KarazhanAttumenTheHuntsmanStackBehindAction::isUseful()
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* boss = karazhanHelper.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return boss && !(botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot);
}

bool KarazhanMoroesMarkTargetAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);

    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = karazhanHelper.GetFirstAliveUnit({dorothea, catriona, keira, rafe, robin, crispin});

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanMaidenOfVirtuePositionBossAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    Unit* healer = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsHeal(member) || !member->HasAura(SPELL_REPENTANCE))
            {
                continue;
            }
            healer = member;
            break;
        }
    }

    if (healer)
    {
        float angle = healer->GetOrientation();
        float targetX = healer->GetPositionX() + cos(angle) * 6.0f;
        float targetY = healer->GetPositionY() + sin(angle) * 6.0f;
        float targetZ = healer->GetPositionZ();
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    const float maxDistance = 3.0f;
    const float distanceToBossPosition = boss->GetExactDist2d(KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION);

    if (distanceToBossPosition > maxDistance)
    {
        float dX = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionX() - boss->GetPositionX();
        float dY = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionY() - boss->GetPositionY();
        float mX = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
        float mY = KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(bot->GetMapId(), mX, mY,
                          bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KarazhanMaidenOfVirtuePositionBossAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return boss && botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}

bool KarazhanMaidenOfVirtuePositionRangedAction::Execute(Event event)
{
    int maxIndex = 7;
    int index = 0;

    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);

        if (!member || !botAI->IsRanged(member->ToPlayer()))
        {
            continue;
        }

        if (member == bot)
            break;

        if (index >= maxIndex)
        {
            index = 0;
            continue;
        }
        index++;
    }

    float distance = bot->GetExactDist2d(KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index]);
    const float maxDistance = 2.0f;
    if (distance > maxDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return MoveTo(bot->GetMapId(), KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index].GetPositionX(),
                      KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[index].GetPositionY(), bot->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KarazhanMaidenOfVirtuePositionRangedAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return boss && botAI->IsRanged(bot);
}

bool KarazhanBigBadWolfPositionBossAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    const float maxDistance = 3.0f;
    const float distanceToBossPosition = boss->GetExactDist2d(KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION);

    if (distanceToBossPosition > maxDistance)
    {
        float dX = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionX() - boss->GetPositionX();
        float dY = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionY() - boss->GetPositionY();

        float mX = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
        float mY = KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;

        float moveDistance = bot->GetExactDist2d(mX, mY);
        if (moveDistance < 0.5f)
        {
            return false;
        }

        return MoveTo(bot->GetMapId(), mX, mY, bot->GetPositionZ(), false, false, false, false,
                     MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KarazhanBigBadWolfPositionBossAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return boss && botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot && 
           !bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool KarazhanBigBadWolfRunAwayAction::Execute(Event event)
{
    constexpr float threshold = 1.0f;
    Position target = KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[currentIndex];

    while (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < threshold)
    {
        currentIndex = (currentIndex + 1) % 4;
        target = KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[currentIndex];
    }

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);

    return MoveTo(bot->GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool KarazhanBigBadWolfRunAwayAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return boss && bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool KarazhanRomuloAndJulianneMarkTargetAction::Execute(Event event)
{
    Unit* target = nullptr;
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");

    const int maxPctDifference = 10;
    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() || julianne->GetHealthPct() < 1.0f)
    {
        target = romulo;
    }
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() || romulo->GetHealthPct() < 1.0f)
    {
        target = julianne;
    }
    if (!target)
    {
        return false;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanWizardOfOzMarkTargetAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");
    Unit* target = karazhanHelper.GetFirstAliveUnit({dorothee, tito, roar, strawman, tinhead, crone});

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanWizardOfOzScorchStrawmanAction::Execute(Event event)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    if (!strawman || !strawman->IsAlive() || bot->getClass() != CLASS_MAGE)
    {
        return false;
    }

    if (botAI->CanCastSpell("scorch", strawman))
    {
        botAI->CastSpell("scorch", strawman);
    }

    return false;
}

bool KarazhanTheCuratorMarkTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");
    if (!target || !target->IsAlive())
    {
        return false;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanTheCuratorPositionBossAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");
    const float maxDistance = 3.0f;
    const float distanceToBossPosition = boss->GetExactDist2d(KARAZHAN_THE_CURATOR_BOSS_POSITION);

    if (distanceToBossPosition > maxDistance)
    {
        float dX = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionX() - boss->GetPositionX();
        float dY = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionY() - boss->GetPositionY();
        float mX = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionX() + (dX / distanceToBossPosition) * maxDistance;
        float mY = KARAZHAN_THE_CURATOR_BOSS_POSITION.GetPositionY() + (dY / distanceToBossPosition) * maxDistance;
        {
            return MoveTo(bot->GetMapId(), mX, mY,
                          bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true,
                          false);
        }
    }

    return false;
}

bool KarazhanTheCuratorPositionBossAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

    return boss && botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}

bool KarazhanTheCuratorSpreadRangedAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    const float minDistance = 5.0f;
    Unit* nearestPlayer = karazhanHelper.GetNearestPlayerInRadius(minDistance);

    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

bool KarazhanTheCuratorSpreadRangedAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

    return boss && botAI->IsRanged(bot);
}

bool KarazhanTerestianIllhoofMarkTargetAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "terestian illhoof");
    if (!boss)
    {
        return false;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_DEMON_CHAINS);
    if (!target || !target->IsAlive()) 
    {
        target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_KILREK);
        if (!target || !target->IsAlive()) 
        {
            target = boss;
        }
    }
    karazhanHelper.MarkTargetWithSkull(target);
    
    return false;
}

bool KarazhanShadeOfAranArcaneExplosionRunAwayAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    const float safeDistance = 20.0f;
    const float distance = bot->GetDistance2d(boss);

    if (distance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return MoveAway(boss, safeDistance - distance);
    }

    return false;
}

bool KarazhanShadeOfAranArcaneExplosionRunAwayAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    return boss && boss->IsAlive() && boss->HasUnitState(UNIT_STATE_CASTING) &&
           boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION);
}

bool KarazhanShadeOfAranFlameWreathStopMovementAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    if (karazhanHelper.IsFlameWreathActive())
    {
        AI_VALUE(LastMovement&, "last movement").Set(nullptr);
        bot->GetMotionMaster()->Clear();
        if (bot->isMoving())
        {
            bot->StopMoving();
        }
        return true;
    }
    
    return false;
}

bool KarazhanShadeOfAranMarkConjuredElementalAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    Unit* target = karazhanHelper.GetFirstAliveUnitByEntry(NPC_CONJURED_ELEMENTAL);

    if (!boss || !boss->IsAlive() || 
        !target || !target->IsAlive() || target->HasAura(SPELL_WARLOCK_BANISH))
    {
        return false;
    }

    karazhanHelper.MarkTargetWithSkull(target);

    return false;
}

bool KarazhanShadeOfAranSpreadRangedAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    const float maxBossDistance = 12.0f;
    float bossDistance = bot->GetExactDist2d(boss);
    if (bossDistance > maxBossDistance)
    {
        float dX = bot->GetPositionX() - boss->GetPositionX();
        float dY = bot->GetPositionY() - boss->GetPositionY();
        float length = std::sqrt(dX * dX + dY * dY);
        dX /= length;
        dY /= length;
        float tX = boss->GetPositionX() + dX * maxBossDistance;
        float tY = boss->GetPositionY() + dY * maxBossDistance;
        {
            return MoveTo(bot->GetMapId(), tX, tY, bot->GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT);
        }
    }

    const float minDistance = 5.0f;
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* nearestPlayer = karazhanHelper.GetNearestPlayerInRadius(minDistance);
    if (nearestPlayer)
    {
        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

bool KarazhanShadeOfAranSpreadRangedAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    RaidKarazhanHelpers karazhanHelper(botAI);

    return boss && boss->IsAlive() && botAI->IsRanged(bot) && !karazhanHelper.IsFlameWreathActive() &&
           !(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION));
}

// One tank bot per phase will dance in and out of the red beam (5 seconds in, 5 seconds out)
// Tank bots will ignore void zones--their positioning is too important
bool KarazhanNetherspiteBlockRedBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);

    RaidKarazhanHelpers karazhanHelper(botAI);
    static std::map<ObjectGuid, bool> wasBlockingRedBeam;
    ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    bool isBlockingNow = (bot == redBlocker);
    bool wasBlocking = wasBlockingRedBeam[botGuid];

    Position beamPos = karazhanHelper.GetPositionOnBeam(boss, redPortal, 18.0f);

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> ph;
            ph["%player"] = bot->GetName();
            std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
                "netherspite_beam_blocking_red", "%player is moving to block the red beam!", ph);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        wasBlockingRedBeam[botGuid] = true;

        uint32 intervalSecs = 5;

        if (beamMoveTimes[botGuid] == 0)
        {
            beamMoveTimes[botGuid] = time(nullptr);
            lastBeamMoveSideways[botGuid] = false;
        }
        if (time(nullptr) - beamMoveTimes[botGuid] >= intervalSecs)
        {
            lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
            beamMoveTimes[botGuid] = time(nullptr);
        }
        if (!lastBeamMoveSideways[botGuid]) 
        {
            return MoveTo(bot->GetMapId(), beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(), 
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        } 
        else 
        {
            float bx = boss->GetPositionX();
            float by = boss->GetPositionY();
            float px = redPortal->GetPositionX();
            float py = redPortal->GetPositionY();
            float dx = px - bx;
            float dy = py - by;
            float length = sqrt(dx*dx + dy*dy);
            if (length == 0.0f)
            {
                return false;
            }

            dx /= length;
            dy /= length;
            float perpDx = -dy;
            float perpDy = dx;
            float sideX = beamPos.GetPositionX() + perpDx * 3.0f;
            float sideY = beamPos.GetPositionY() + perpDy * 3.0f;
            float sideZ = beamPos.GetPositionZ();

            return MoveTo(bot->GetMapId(), sideX, sideY, sideZ, false, false, false, true, 
                          MovementPriority::MOVEMENT_FORCED);
        }
    }

    wasBlockingRedBeam[botGuid] = false;
    return false;
}

bool KarazhanNetherspiteBlockRedBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);

    ObjectGuid botGuid = bot->GetGUID();
    static std::map<ObjectGuid, bool> lastBossBanishState;
    bool bossIsBanished = boss && boss->HasAura(SPELL_NETHERSPITE_BANISHED);

    if (lastBossBanishState[botGuid] != bossIsBanished)
    {
        if (!bossIsBanished)
        {
            beamMoveTimes[botGuid] = 0;
            lastBeamMoveSideways[botGuid] = false;
        }
        lastBossBanishState[botGuid] = bossIsBanished;
    }

    return boss && redPortal && !bossIsBanished;
}

// Two non-Rogue/Warrior DPS bots will block the blue beam for each phase (swap at 26 debuff stacks)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool KarazhanNetherspiteBlockBlueBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);

    RaidKarazhanHelpers karazhanHelper(botAI);
    static std::map<ObjectGuid, bool> wasBlockingBlueBeam;
    ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    bool isBlockingNow = (bot == blueBlocker);
    bool wasBlocking = wasBlockingBlueBeam[botGuid];

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> ph;
        ph["%player"] = bot->GetName();
        std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
            "netherspite_beam_leaving_blue", "%player is leaving the blue beam--next blocker up!", ph);
        bot->Yell(text, LANG_UNIVERSAL);
        wasBlockingBlueBeam[botGuid] = false;
        
        return false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> ph;
            ph["%player"] = bot->GetName();
            std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
                "netherspite_beam_blocking_blue", "%player is moving to block the blue beam!", ph);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        wasBlockingBlueBeam[botGuid] = true;

        std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();
        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float bz = boss->GetPositionZ();
        float px = bluePortal->GetPositionX();
        float py = bluePortal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length == 0.0f)
        {
            return false;
        }

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;
        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f) 
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            bool outsideAllVoidZones = true;
            for (Unit* voidZone : voidZones) 
            {
                float voidZoneDist = sqrt(pow(candidateX - voidZone->GetPositionX(), 2) + 
                                          pow(candidateY - voidZone->GetPositionY(), 2));
                if (voidZoneDist < 4.0f) 
                {
                    outsideAllVoidZones = false;
                    break;
                }
            }
            if (!outsideAllVoidZones)
            {
                continue;
            }
            float distToIdeal = fabs(dist - 18.0f);
            if (!found || distToIdeal < bestDist) 
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }
        if (found) 
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }

        return false;
    }

    wasBlockingBlueBeam[botGuid] = false;
    return false;
}

bool KarazhanNetherspiteBlockBlueBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);

    return boss && bluePortal && !boss->HasAura(SPELL_NETHERSPITE_BANISHED);
}

// Two healer bots will block the green beam for each phase (swap at 26 debuff stacks)
// OR one rogue or DPS warrior bot will block the green beam for an entire phase (if they begin the phase as the blocker)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool KarazhanNetherspiteBlockGreenBeamAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    RaidKarazhanHelpers karazhanHelper(botAI);
    static std::map<ObjectGuid, bool> wasBlockingGreenBeam;
    ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    bool isBlockingNow = (bot == greenBlocker);
    bool wasBlocking = wasBlockingGreenBeam[botGuid];

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> ph;
        ph["%player"] = bot->GetName();
        std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
            "netherspite_beam_leaving_green", "%player is leaving the green beam--next blocker up!", ph);
        bot->Yell(text, LANG_UNIVERSAL);
        wasBlockingGreenBeam[botGuid] = false;

        return false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> ph;
            ph["%player"] = bot->GetName();
            std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
                "netherspite_beam_blocking_green", "%player is moving to block the green beam!", ph);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        wasBlockingGreenBeam[botGuid] = true;

        std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();
        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float bz = boss->GetPositionZ();
        float px = greenPortal->GetPositionX();
        float py = greenPortal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length == 0.0f)
        {
            return false;
        }

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;
        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f) 
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            bool outsideAllVoidZones = true;
            for (Unit* voidZone : voidZones) 
            {
                float voidZoneDist = sqrt(pow(candidateX - voidZone->GetPositionX(), 2) + 
                                          pow(candidateY - voidZone->GetPositionY(), 2));
                if (voidZoneDist < 4.0f) 
                {
                    outsideAllVoidZones = false;
                    break;
                }
            }
            if (!outsideAllVoidZones)
            {
                continue;
            }
            float distToIdeal = fabs(dist - 18.0f);
            if (!found || distToIdeal < bestDist) 
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }
        if (found) 
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), 
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }

        return false;
    }

    wasBlockingGreenBeam[botGuid] = false;
    return false;
}

bool KarazhanNetherspiteBlockGreenBeamAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    return boss && greenPortal && !boss->HasAura(SPELL_NETHERSPITE_BANISHED);
}

// All bots not currently blocking a beam will avoid beams and void zones
bool KarazhanNetherspiteAvoidBeamAndVoidZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();
    bool nearVoidZone = false;
    for (Unit* vz : voidZones)
    {
        if (bot->GetExactDist2d(vz) < 4.0f)
        {
            nearVoidZone = true;
            break;
        }
    }
    struct BeamAvoid { Unit* portal; float minDist, maxDist; };
    std::vector<BeamAvoid> beams;
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);
    if (redPortal) 
    {
        float bx = boss->GetPositionX(), by = boss->GetPositionY();
        float px = redPortal->GetPositionX(), py = redPortal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        beams.push_back({redPortal, 0.0f, length});
    }
    if (bluePortal) 
    {
        float bx = boss->GetPositionX(), by = boss->GetPositionY();
        float px = bluePortal->GetPositionX(), py = bluePortal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        beams.push_back({bluePortal, 0.0f, length});
    }
    if (greenPortal) 
    {
        float bx = boss->GetPositionX(), by = boss->GetPositionY();
        float px = greenPortal->GetPositionX(), py = greenPortal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        beams.push_back({greenPortal, 0.0f, length});
    }
    bool nearBeam = false;
    for (const auto& beam : beams)
    {
        float bx = boss->GetPositionX(), by = boss->GetPositionY();
        float px = beam.portal->GetPositionX(), py = beam.portal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length == 0.0f)
        {
            continue;
        }
        dx /= length; dy /= length;
        float botdx = bot->GetPositionX() - bx, botdy = bot->GetPositionY() - by;
        float t = (botdx * dx + botdy * dy);
        float beamX = bx + dx * t, beamY = by + dy * t;
        float distToBeam = sqrt(pow(bot->GetPositionX() - beamX, 2) + pow(bot->GetPositionY() - beamY, 2));
        if (distToBeam < 5.0f && t > beam.minDist && t < beam.maxDist)
        {
            nearBeam = true;
            break;
        }
    }
    if (!nearVoidZone && !nearBeam)
    {
        return false;
    }

    const float minMoveDist = 2.0f, maxSearchDist = 30.0f, stepAngle = M_PI/18.0f, stepDist = 0.5f;
    float bossZ = boss->GetPositionZ();
    Position bestCandidate;
    float bestDist = 0.0f;
    bool found = false;
    for (float angle = 0; angle < 2 * M_PI; angle += stepAngle)
    {
        for (float dist = 2.0f; dist <= maxSearchDist; dist += stepDist)
        {
            float cx = bot->GetPositionX() + cos(angle) * dist;
            float cy = bot->GetPositionY() + sin(angle) * dist;
            float cz = bossZ;
            if (std::any_of(voidZones.begin(), voidZones.end(), [&](Unit* vz){ return Position(cx, cy, cz).GetExactDist2d(vz) < 4.0f; }))
            {
                continue;
            }
            bool tooCloseToBeam = false;
            for (const auto& beam : beams)
            {
                float bx = boss->GetPositionX(), by = boss->GetPositionY();
                float px = beam.portal->GetPositionX(), py = beam.portal->GetPositionY();
                float dx = px - bx, dy = py - by;
                float length = sqrt(dx*dx + dy*dy);
                if (length == 0.0f) 
                {
                    continue;
                }
                dx /= length; dy /= length;
                float botdx = cx - bx, botdy = cy - by;
                float t = (botdx * dx + botdy * dy);
                float beamX = bx + dx * t, beamY = by + dy * t;
                float distToBeam = sqrt(pow(cx - beamX, 2) + pow(cy - beamY, 2));
                if (distToBeam < 5.0f && t > beam.minDist && t < beam.maxDist)
                {
                    tooCloseToBeam = true;
                    break;
                }
            }
            if (tooCloseToBeam)
            {
                continue;
            }
            float moveDist = sqrt(pow(cx - bot->GetPositionX(), 2) + pow(cy - bot->GetPositionY(), 2));
            if (moveDist < minMoveDist)
            {
                continue;
            }
            if (!found || moveDist < bestDist) 
            {
                bestCandidate = Position(cx, cy, cz);
                bestDist = moveDist;
                found = true;
            }
        }
    }
    if (found && karazhanHelper.IsSafePosition(bestCandidate.GetPositionX(), 
        bestCandidate.GetPositionY(), bestCandidate.GetPositionZ(), 
        voidZones, 4.0f))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return MoveTo(bot->GetMapId(), bestCandidate.GetPositionX(), bestCandidate.GetPositionY(), 
                      bestCandidate.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool KarazhanNetherspiteAvoidBeamAndVoidZoneAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || boss->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        return false;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    if (bot == redBlocker || bot == blueBlocker || bot == greenBlocker)
    {
        return false;
    }

    return true;
}

bool KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event event)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();

    for (Unit* vz : voidZones)
    {
        if (vz->GetEntry() == NPC_VOID_ZONE && bot->GetExactDist2d(vz) < 4.0f)
        {
            return FleePosition(vz->GetPosition(), 4.0f);
        }
    }

    return false;
}

bool KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        return false;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();
    for (Unit* vz : voidZones) 
    {
        if (bot->GetExactDist2d(vz) < 4.0f)
        {
            return true;
        }
    }

    return false;
}

bool KarazhanPrinceMalchezaarNonTankAvoidHazardAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    RaidKarazhanHelpers karazhanHelper(botAI);
    std::vector<Unit*> infernals = karazhanHelper.GetSpawnedInfernals();

    const float minSafeBossDistance = 34.0f;
    const float maxSafeBossDistance = 60.0f;
    const float safeInfernalDistance = 23.0f;
    const float stepSize = 0.5f;
    const int numAngles = 64;
    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float bossX = boss->GetPositionX();
    float bossY = boss->GetPositionY();
    float bossZ = boss->GetPositionZ();
    float bestMoveDist = std::numeric_limits<float>::max();
    float bestDestX = 0.0f, bestDestY = 0.0f, bestDestZ = bz;
    bool found = false;

    if (bot->HasAura(SPELL_ENFEEBLE))
    {
        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2 * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);
            for (float dist = minSafeBossDistance; dist <= maxSafeBossDistance; dist += stepSize)
            {
                float x = bossX + dx * dist;
                float y = bossY + dy * dist;
                float destZ = bossZ;
                float destX = x, destY = y, destZ2 = destZ;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ2, true))
                {
                    continue;
                }
                float distFromBoss = sqrt(pow(destX - bossX, 2) + pow(destY - bossY, 2));
                if (distFromBoss < minSafeBossDistance)
                {
                    continue;
                }
                bool pathSafe = karazhanHelper.IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ2),
                                                                  infernals, safeInfernalDistance, stepSize);
                float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                if (pathSafe && moveDist < bestMoveDist)
                {
                    bestMoveDist = moveDist;
                    bestDestX = destX;
                    bestDestY = destY;
                    bestDestZ = destZ2;
                    found = true;
                }
            }
        }
        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, true, 
                          MovementPriority::MOVEMENT_FORCED);
        }

        return false;
    }

    if (!bot->HasAura(SPELL_ENFEEBLE))
    {
        bool nearInfernal = false;
        for (Unit* infernal : infernals)
        {
            float infernalDist = sqrt(pow(bx - infernal->GetPositionX(), 2) + pow(by - infernal->GetPositionY(), 2));
            if (infernalDist < safeInfernalDistance)
            {
                nearInfernal = true;
                break;
            }
        }
        if (nearInfernal)
        {
            float bestMoveDist = std::numeric_limits<float>::max();
            float bestDestX = bx, bestDestY = by, bestDestZ = bz;
            bool found = false;
            for (int i = 0; i < numAngles; ++i)
            {
                float angle = (2 * M_PI * i) / numAngles;
                float dx = cos(angle);
                float dy = sin(angle);
                for (float dist = stepSize; dist <= maxSafeBossDistance; dist += stepSize)
                {
                    float x = bossX + dx * dist;
                    float y = bossY + dy * dist;
                    float destZ = bossZ;
                    float destX = x, destY = y, destZ2 = destZ;
                    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bossX, bossY, bossZ, destX, destY, destZ2, true))
                    {
                        continue;
                    }
                    bool destSafe = true;
                    for (Unit* infernal : infernals)
                    {
                        float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                        if (infernalDist < safeInfernalDistance)
                        {
                            destSafe = false;
                            break;
                        }
                    }
                    if (!destSafe)
                        continue;
                    float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                    if (moveDist < bestMoveDist)
                    {
                        bestMoveDist = moveDist;
                        bestDestX = destX;
                        bestDestY = destY;
                        bestDestZ = destZ2;
                        found = true;
                    }
                }
            }
            if (found)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(false);

                return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, true, 
                              MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool KarazhanPrinceMalchezaarNonTankAvoidHazardAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    
    return boss && !(botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot);
}

bool KarazhanPrinceMalchezaarTankAvoidHazardAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    RaidKarazhanHelpers karazhanHelper(botAI);
    std::vector<Unit*> infernals = karazhanHelper.GetSpawnedInfernals();

    const float safeInfernalDistance = 28.0f;
    const float stepSize = 0.5f;
    const int numAngles = 64;
    const float maxSampleDist = 60.0f;
    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float infernalDist = sqrt(pow(bx - infernal->GetPositionX(), 2) + pow(by - infernal->GetPositionY(), 2));
        if (infernalDist < safeInfernalDistance)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestMoveDist = std::numeric_limits<float>::max();
    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2 * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);
            for (float dist = stepSize; dist <= maxSampleDist; dist += stepSize)
            {
                float x = bx + dx * dist;
                float y = by + dy * dist;
                float z = bz;

                float destX = x, destY = y, destZ = z;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ, true))
                    continue;

                bool destSafe = true;
                for (Unit* infernal : infernals)
                {
                    float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                    if (infernalDist < safeInfernalDistance)
                    {
                        destSafe = false;
                        break;
                    }
                }
                if (!destSafe)
                    continue;

                bool pathSafe = karazhanHelper.IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ), 
                                                                 infernals, safeInfernalDistance, stepSize);
                float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                if (pathSafe && moveDist < bestMoveDist)
                {
                    bestMoveDist = moveDist;
                    bestDestX = destX;
                    bestDestY = destY;
                    bestDestZ = destZ;
                    found = true;
                }
            }
        }
        if (!found)
        {
            for (int i = 0; i < numAngles; ++i)
            {
                float angle = (2 * M_PI * i) / numAngles;
                float dx = cos(angle);
                float dy = sin(angle);
                for (float dist = stepSize; dist <= maxSampleDist; dist += stepSize)
                {
                    float x = bx + dx * dist;
                    float y = by + dy * dist;
                    float z = bz;

                    float destX = x, destY = y, destZ = z;
                    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ, true))
                        continue;

                    bool destSafe = true;
                    for (Unit* infernal : infernals)
                    {
                        float infernalDist = sqrt(pow(destX - infernal->GetPositionX(), 2) + pow(destY - infernal->GetPositionY(), 2));
                        if (infernalDist < safeInfernalDistance)
                        {
                            destSafe = false;
                            break;
                        }
                    }
                    float moveDist = sqrt(pow(destX - bx, 2) + pow(destY - by, 2));
                    if (destSafe && moveDist < bestMoveDist)
                    {
                        bestMoveDist = moveDist;
                        bestDestX = destX;
                        bestDestY = destY;
                        bestDestZ = destZ;
                        found = true;
                    }
                }
            }
        }
        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(bot->GetMapId(), bestDestX, bestDestY, bestDestZ, false, false, false, true, 
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool KarazhanPrinceMalchezaarTankAvoidHazardAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    
    return boss && botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot;
}
