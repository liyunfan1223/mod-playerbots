/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObjectContext.h"
#include "Aq20TriggerContext.h"
#include "BTTriggerContext.h"
#include "BWLTriggerContext.h"
#include "ChatTriggerContext.h"
#include "EoETriggerContext.h"
#include "GruulTriggerContext.h"
#include "HyjalTriggerContext.h"
#include "ICCTriggerContext.h"
#include "KaraTriggerContext.h"
#include "MCTriggerContext.h"
#include "MagTriggerContext.h"
#include "NaxxTriggerContext.h"
#include "OSTriggerContext.h"
#include "OnyTriggerContext.h"
#include "RSTriggerContext.h"
#include "SSCTriggerContext.h"
#include "TKTriggerContext.h"
#include "TbcDungeonTriggerContext.h"
#include "TriggerContext.h"
#include "UldTriggerContext.h"
#include "VoATriggerContext.h"
#include "WorldPacketTriggerContext.h"
#include "WotlkDungeonTriggerContext.h"
#include "ZATriggerContext.h"

void AiObjectContext::BuildSharedTriggerContexts(
    SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    triggerContexts.Add(new TriggerContext());
    triggerContexts.Add(new ChatTriggerContext());
    triggerContexts.Add(new WorldPacketTriggerContext());
    triggerContexts.Add(new RaidAq20TriggerContext());
    triggerContexts.Add(new RaidMcTriggerContext());
    triggerContexts.Add(new RaidBwlTriggerContext());
    triggerContexts.Add(new RaidKarazhanTriggerContext());
    triggerContexts.Add(new RaidGruulsLairTriggerContext());
    triggerContexts.Add(new RaidMagtheridonTriggerContext());
    triggerContexts.Add(new RaidNaxxTriggerContext());
    triggerContexts.Add(new RaidSscTriggerContext());
    triggerContexts.Add(new RaidTempestKeepTriggerContext());
    triggerContexts.Add(new RaidHyjalTriggerContext());
    triggerContexts.Add(new RaidBlackTempleTriggerContext());
    triggerContexts.Add(new RaidZulAmanTriggerContext());
    triggerContexts.Add(new RaidOsTriggerContext());
    triggerContexts.Add(new RaidEoETriggerContext());
    triggerContexts.Add(new RaidVoATriggerContext());
    triggerContexts.Add(new RaidUlduarTriggerContext());
    triggerContexts.Add(new RaidOnyxiaTriggerContext());
    triggerContexts.Add(new RaidIccTriggerContext());
    triggerContexts.Add(new RaidRsTriggerContext());
    triggerContexts.Add(new TbcDungeonAuchenaiCryptsTriggerContext());
    triggerContexts.Add(new TbcDungeonSethekkHallsTriggerContext());
    triggerContexts.Add(new TbcDungeonMechanarTriggerContext());
    triggerContexts.Add(new TbcDungeonUnderbogTriggerContext());
    triggerContexts.Add(new TbcDungeonMagistersTerraceTriggerContext());
    triggerContexts.Add(new TbcDungeonHellfireRampartsTriggerContext());
    triggerContexts.Add(new WotlkDungeonUKTriggerContext());
    triggerContexts.Add(new WotlkDungeonNexTriggerContext());
    triggerContexts.Add(new WotlkDungeonANTriggerContext());
    triggerContexts.Add(new WotlkDungeonOKTriggerContext());
    triggerContexts.Add(new WotlkDungeonDTKTriggerContext());
    triggerContexts.Add(new WotlkDungeonVHTriggerContext());
    triggerContexts.Add(new WotlkDungeonGDTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoLTriggerContext());
    triggerContexts.Add(new WotlkDungeonOccTriggerContext());
    triggerContexts.Add(new WotlkDungeonUPTriggerContext());
    triggerContexts.Add(new WotlkDungeonCoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonFoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonPoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonToCTriggerContext());
}
