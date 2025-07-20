#include "Playerbots.h"
#include "RaidKarazhanTriggers.h"
#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"

bool KarazhanAttumenTheHuntsmanTrigger::IsActive()
{
    RaidKarazhanHelpers helpers(botAI);
    Unit* boss = helpers.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN);
    return boss != nullptr;
}

bool KarazhanMoroesTrigger::IsActive()
{
    Unit* moroes = AI_VALUE2(Unit*, "find target", "moroes");
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");

    if ((!moroes || !moroes->IsAlive()) &&
        (!dorothea || !dorothea->IsAlive()) &&
        (!catriona || !catriona->IsAlive()) &&
        (!keira || !keira->IsAlive()) &&
        (!rafe || !rafe->IsAlive()) &&
        (!robin || !robin->IsAlive()) &&
        (!crispin || !crispin->IsAlive()))
        return false;

    return true;
}

bool KarazhanMaidenOfVirtueTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    if (!boss || !boss->IsAlive())
        return false;

    return true;
}

bool KarazhanBigBadWolfTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    if (!boss || !boss->IsAlive())
        return false;

    return true;
}

bool KarazhanRomuloAndJulianneTrigger::IsActive()
{
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");

    if (!julianne || !julianne->IsAlive() || !romulo || !romulo->IsAlive())
        return false;

    return true;
}

bool KarazhanWizardOfOzTrigger::IsActive()
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");

    if (( !dorothee || !dorothee->IsAlive() ) &&
        ( !tito || !tito->IsAlive() ) &&
        ( !roar || !roar->IsAlive() ) &&
        ( !strawman || !strawman->IsAlive() ) &&
        ( !tinhead || !tinhead->IsAlive() ) &&
        ( !crone || !crone->IsAlive() ))
        return false;

    return true;
}

bool KarazhanTheCuratorTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

    if (!boss || !boss->IsAlive())
        return false;

    return true;
}

bool KarazhanTerestianIllhoofTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "terestian illhoof");

    if (!boss || !boss->IsAlive())
        return false;

    return true;
}

bool KarazhanShadeOfAranTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    if (!boss)
        return false;

    return true;
}

bool KarazhanNetherspiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    if (!boss || !boss->IsAlive())
        return false;

    return true;
}

bool KarazhanPrinceMalchezaarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    if (!boss || !boss->IsAlive())
        return false;

    return true;
}
