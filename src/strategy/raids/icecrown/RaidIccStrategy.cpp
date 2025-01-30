#include "RaidIccStrategy.h"

#include "RaidIccMultipliers.h"

void RaidIccStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //Lord Marrogwar
    triggers.push_back(new TriggerNode("icc lm tank position",
        NextAction::array(0, new NextAction("icc lm tank position", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc spike near",
        NextAction::array(0, new NextAction("icc spike", ACTION_RAID + 5), nullptr)));

    //Lady Deathwhisper
    triggers.push_back(new TriggerNode("icc dark reckoning",
        NextAction::array(0, new NextAction("icc dark reckoning", ACTION_MOVE + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc ranged position lady deathwhisper", 
        NextAction::array(0, new NextAction("icc ranged position lady deathwhisper", ACTION_MOVE + 2), nullptr)));

    triggers.push_back(new TriggerNode("icc adds lady deathwhisper",
        NextAction::array(0, new NextAction("icc adds lady deathwhisper", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("icc shade lady deathwhisper",
        NextAction::array(0, new NextAction("icc shade lady deathwhisper", ACTION_MOVE + 5), nullptr)));
    
    //Gunship Battle
    triggers.push_back(new TriggerNode("icc rotting frost giant tank position",
        NextAction::array(0, new NextAction("icc rotting frost giant tank position", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc gunship cannon near",
        NextAction::array(0, new NextAction("icc gunship enter cannon", ACTION_RAID + 2), nullptr)));

    triggers.push_back( new TriggerNode("icc in cannon",
        NextAction::array(0, new NextAction("icc cannon fire", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("icc gunship teleport ally",
        NextAction::array(0, new NextAction("icc gunship teleport ally", ACTION_RAID + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("icc gunship teleport horde",
        NextAction::array(0, new NextAction("icc gunship teleport horde", ACTION_RAID + 4), nullptr)));

    //DBS
    triggers.push_back(new TriggerNode("icc dbs tank position",
        NextAction::array(0, new NextAction("icc dbs tank position", ACTION_RAID + 3), nullptr)));
    
    triggers.push_back(new TriggerNode("icc dbs main tank rune of blood",
        NextAction::array(0, new NextAction("taunt spell", ACTION_EMERGENCY + 4), nullptr)));

    triggers.push_back(new TriggerNode("icc adds dbs",
        NextAction::array(0, new NextAction("icc adds dbs", ACTION_RAID + 5), nullptr)));
    
    //DOGS
    triggers.push_back(new TriggerNode("icc stinky precious main tank mortal wound",
        NextAction::array(0, new NextAction("taunt spell", ACTION_EMERGENCY + 4), nullptr)));
    
    //FESTERGUT
    triggers.push_back(new TriggerNode("icc festergut tank position",
        NextAction::array(0, new NextAction("icc festergut tank position", ACTION_MOVE + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("icc festergut main tank gastric bloat",
        NextAction::array(0, new NextAction("taunt spell", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("icc festergut spore",
        NextAction::array(0, new NextAction("icc festergut spore", ACTION_MOVE + 5), nullptr)));
    
    //ROTFACE
    triggers.push_back(new TriggerNode("icc rotface tank position",
        NextAction::array(0, new NextAction("icc rotface tank position", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc rotface group position",
        NextAction::array(0, new NextAction("icc rotface group position", ACTION_RAID + 6), nullptr)));

    triggers.push_back(new TriggerNode("icc rotface move away from explosion",
        NextAction::array(0, new NextAction("icc rotface move away from explosion", ACTION_RAID +7), nullptr)));

    //PP
    triggers.push_back(new TriggerNode("icc putricide volatile ooze",
        NextAction::array(0, new NextAction("icc putricide volatile ooze", ACTION_RAID + 4), nullptr)));

    triggers.push_back(new TriggerNode("icc putricide gas cloud",
        NextAction::array(0, new NextAction("icc putricide gas cloud", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc putricide growing ooze puddle",
        NextAction::array(0, new NextAction("icc putricide growing ooze puddle", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("icc putricide main tank mutated plague",
        NextAction::array(0, new NextAction("taunt spell", ACTION_RAID + 6), nullptr)));

    triggers.push_back(new TriggerNode("icc putricide malleable goo",
        NextAction::array(0, new NextAction("avoid malleable goo", ACTION_RAID + 2), nullptr)));

    //BPC
    triggers.push_back(new TriggerNode("icc bpc keleseth tank",
        NextAction::array(0, new NextAction("icc bpc keleseth tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("icc bpc nucleus",
        NextAction::array(0, new NextAction("icc bpc nucleus", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("icc bpc main tank",
        NextAction::array(0, new NextAction("icc bpc main tank", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("icc bpc empowered vortex",
        NextAction::array(0, new NextAction("icc bpc empowered vortex", ACTION_RAID + 4), nullptr)));

    triggers.push_back(new TriggerNode("icc bpc kinetic bomb",
        NextAction::array(0, new NextAction("icc bpc kinetic bomb", ACTION_RAID + 6), nullptr)));
    
    //BQL
    triggers.push_back(new TriggerNode("icc bql tank position",
        NextAction::array(0, new NextAction("icc bql tank position", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("icc bql pact of darkfallen",
        NextAction::array(0, new NextAction("icc bql pact of darkfallen", ACTION_RAID +1), nullptr)));

    triggers.push_back(new TriggerNode("icc bql vampiric bite",
        NextAction::array(0, new NextAction("icc bql vampiric bite", ACTION_EMERGENCY + 5), nullptr)));

    //VDW
    triggers.push_back(new TriggerNode("icc valkyre spear",
        NextAction::array(0, new NextAction("icc valkyre spear", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc sister svalna",
        NextAction::array(0, new NextAction("icc sister svalna", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc valithria portal",
        NextAction::array(0, new NextAction("icc valithria portal", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("icc valithria heal", 
        NextAction::array(0, new NextAction("icc valithria heal", ACTION_RAID+1), nullptr)));

    triggers.push_back(new TriggerNode("icc valithria dream cloud",
        NextAction::array(0, new NextAction("icc valithria dream cloud", ACTION_RAID + 4), nullptr)));

    //SINDRAGOSA
    triggers.push_back(new TriggerNode("icc sindragosa tank position",
        NextAction::array(0, new NextAction("icc sindragosa tank position", ACTION_RAID + 1), nullptr)));   

    triggers.push_back(new TriggerNode("icc sindragosa frost beacon",
        NextAction::array(0, new NextAction("icc sindragosa frost beacon", ACTION_RAID + 5), nullptr)));    

    triggers.push_back(new TriggerNode("icc sindragosa blistering cold",
        NextAction::array(0, new NextAction("icc sindragosa blistering cold", ACTION_EMERGENCY + 4), nullptr))); 

    triggers.push_back(new TriggerNode("icc sindragosa unchained magic",
        NextAction::array(0, new NextAction("icc sindragosa unchained magic", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("icc sindragosa chilled to the bone",
        NextAction::array(0, new NextAction("icc sindragosa chilled to the bone", ACTION_RAID + 2), nullptr))); 

    triggers.push_back(new TriggerNode("icc sindragosa mystic buffet",
        NextAction::array(0, new NextAction("icc sindragosa mystic buffet", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("icc sindragosa main tank mystic buffet",
        NextAction::array(0, new NextAction("taunt spell", ACTION_EMERGENCY + 3), nullptr)));

    triggers.push_back(new TriggerNode("icc sindragosa frost bomb",
        NextAction::array(0, new NextAction("icc sindragosa frost bomb", ACTION_RAID + 7), nullptr)));

    triggers.push_back(new TriggerNode("icc sindragosa tank swap position",
        NextAction::array(0, new NextAction("icc sindragosa tank swap position", ACTION_EMERGENCY + 2), nullptr)));

    //LICH KING
    triggers.push_back(new TriggerNode("icc lich king shadow trap",
        NextAction::array(0, new NextAction("icc lich king shadow trap", ACTION_RAID + 6), nullptr)));

    triggers.push_back(new TriggerNode("icc lich king necrotic plague",
        NextAction::array(0, new NextAction("icc lich king necrotic plague", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("icc lich king winter",
        NextAction::array(0, new NextAction("icc lich king winter", ACTION_RAID +5), nullptr)));

    triggers.push_back(new TriggerNode("icc lich king adds",
        NextAction::array(0, new NextAction("icc lich king adds", ACTION_RAID +2), nullptr)));
}

void RaidIccStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IccLadyDeathwhisperMultiplier(botAI));
    multipliers.push_back(new IccAddsDbsMultiplier(botAI));
    multipliers.push_back(new IccDogsMultiplier(botAI));
    multipliers.push_back(new IccFestergutMultiplier(botAI));
    multipliers.push_back(new IccRotfaceMultiplier(botAI));
    //multipliers.push_back(new IccRotfaceGroupPositionMultiplier(botAI));
    multipliers.push_back(new IccAddsPutricideMultiplier(botAI));
    multipliers.push_back(new IccBpcAssistMultiplier(botAI));
    multipliers.push_back(new IccBqlPactOfDarkfallenMultiplier(botAI));
    multipliers.push_back(new IccBqlVampiricBiteMultiplier(botAI));
    multipliers.push_back(new IccValithriaDreamCloudMultiplier(botAI));
    multipliers.push_back(new IccSindragosaTankPositionMultiplier(botAI));
    multipliers.push_back(new IccSindragosaFrostBeaconMultiplier(botAI));
    //multipliers.push_back(new IccSindragosaFlyingMultiplier(botAI));
    multipliers.push_back(new IccSindragosaMysticBuffetMultiplier(botAI));
    multipliers.push_back(new IccSindragosaBlisteringColdPriorityMultiplier(botAI));
    multipliers.push_back(new IccSindragosaFrostBombMultiplier(botAI));
    multipliers.push_back(new IccLichKingNecroticPlagueMultiplier(botAI));
    multipliers.push_back(new IccLichKingAddsMultiplier(botAI));
}
