/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ActionContext.h"
#include "AiObjectContext.h"
#include "Aq20ActionContext.h"
#include "BTActionContext.h"
#include "BWLActionContext.h"
#include "ChatActionContext.h"
#include "EoEActionContext.h"
#include "GruulActionContext.h"
#include "HyjalActionContext.h"
#include "ICCActionContext.h"
#include "KaraActionContext.h"
#include "MCActionContext.h"
#include "MagActionContext.h"
#include "NaxxActionContext.h"
#include "OSActionContext.h"
#include "OnyActionContext.h"
#include "RSActionContext.h"
#include "SSCActionContext.h"
#include "TKActionContext.h"
#include "TbcDungeonActionContext.h"
#include "UldActionContext.h"
#include "VoAActionContext.h"
#include "WorldPacketActionContext.h"
#include "WotlkDungeonActionContext.h"
#include "ZAActionContext.h"

void AiObjectContext::BuildSharedActionContexts(
    SharedNamedObjectContextList<Action>& actionContexts)
{
    actionContexts.Add(new ActionContext());
    actionContexts.Add(new ChatActionContext());
    actionContexts.Add(new WorldPacketActionContext());
    actionContexts.Add(new RaidAq20ActionContext());
    actionContexts.Add(new RaidMcActionContext());
    actionContexts.Add(new RaidBwlActionContext());
    actionContexts.Add(new RaidKarazhanActionContext());
    actionContexts.Add(new RaidGruulsLairActionContext());
    actionContexts.Add(new RaidMagtheridonActionContext());
    actionContexts.Add(new RaidSscActionContext());
    actionContexts.Add(new RaidTempestKeepActionContext());
    actionContexts.Add(new RaidHyjalActionContext());
    actionContexts.Add(new RaidBlackTempleActionContext());
    actionContexts.Add(new RaidZulAmanActionContext());
    actionContexts.Add(new RaidNaxxActionContext());
    actionContexts.Add(new RaidOsActionContext());
    actionContexts.Add(new RaidEoEActionContext());
    actionContexts.Add(new RaidVoAActionContext());
    actionContexts.Add(new RaidUlduarActionContext());
    actionContexts.Add(new RaidOnyxiaActionContext());
    actionContexts.Add(new RaidIccActionContext());
    actionContexts.Add(new RaidRsActionContext());
    actionContexts.Add(new TbcDungeonAuchenaiCryptsActionContext());
    actionContexts.Add(new TbcDungeonSethekkHallsActionContext());
    actionContexts.Add(new TbcDungeonMechanarActionContext());
    actionContexts.Add(new TbcDungeonUnderbogActionContext());
    actionContexts.Add(new TbcDungeonMagistersTerraceActionContext());
    actionContexts.Add(new TbcDungeonHellfireRampartsActionContext());
    actionContexts.Add(new WotlkDungeonUKActionContext());
    actionContexts.Add(new WotlkDungeonNexActionContext());
    actionContexts.Add(new WotlkDungeonANActionContext());
    actionContexts.Add(new WotlkDungeonOKActionContext());
    actionContexts.Add(new WotlkDungeonDTKActionContext());
    actionContexts.Add(new WotlkDungeonVHActionContext());
    actionContexts.Add(new WotlkDungeonGDActionContext());
    actionContexts.Add(new WotlkDungeonHoSActionContext());
    actionContexts.Add(new WotlkDungeonHoLActionContext());
    actionContexts.Add(new WotlkDungeonOccActionContext());
    actionContexts.Add(new WotlkDungeonUPActionContext());
    actionContexts.Add(new WotlkDungeonCoSActionContext());
    actionContexts.Add(new WotlkDungeonFoSActionContext());
    actionContexts.Add(new WotlkDungeonPoSActionContext());
    actionContexts.Add(new WotlkDungeonToCActionContext());
}
