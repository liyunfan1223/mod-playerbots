#include "RaidKarazhanTriggers.h"
#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"
#include "Playerbots.h"

bool KarazhanAttumenTheHuntsmanTrigger::IsActive()
{
    RaidKarazhanHelpers helpers(botAI);
    Unit* boss = helpers.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return boss && boss->IsAlive();
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

    return ((moroes && moroes->IsAlive()) ||
            (dorothea && dorothea->IsAlive()) ||
            (catriona && catriona->IsAlive()) ||
            (keira && keira->IsAlive()) ||
            (rafe && rafe->IsAlive()) ||
            (robin && robin->IsAlive()) ||
            (crispin && crispin->IsAlive()));
}

bool KarazhanMaidenOfVirtueTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maiden of virtue");

    return boss && boss->IsAlive();
}

bool KarazhanBigBadWolfTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");

    return boss && boss->IsAlive();
}

bool KarazhanRomuloAndJulianneTrigger::IsActive()
{
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");

    return julianne && julianne->IsAlive() && romulo && romulo->IsAlive();
}

bool KarazhanWizardOfOzTrigger::IsActive()
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");

    return ((dorothee && dorothee->IsAlive()) ||
            (tito && tito->IsAlive()) ||
            (roar && roar->IsAlive()) ||
            (strawman && strawman->IsAlive()) ||
            (tinhead && tinhead->IsAlive()) ||
            (crone && crone->IsAlive()));
}

bool KarazhanTheCuratorTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the curator");

    return boss && boss->IsAlive();
}

bool KarazhanTerestianIllhoofTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "terestian illhoof");

    return boss && boss->IsAlive();
}

bool KarazhanShadeOfAranTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");

    return boss && boss->IsAlive();
}

bool KarazhanNetherspiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");

    return boss && boss->IsAlive();
}

bool KarazhanPrinceMalchezaarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");

    return boss && boss->IsAlive();
}
