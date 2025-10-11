#include "RaidBuilderCommand.h"
#include "AiFactory.h"

RaidComposition::RoleComposition RaidComposition::GetOptimalComposition(int raidSize)
{
    RoleComposition comp;
    
    int calculatedTanks = static_cast<int>(std::ceil(raidSize * TANK_RATIO));
    comp.tanks = std::max(MIN_TANKS, std::min(MAX_TANKS, calculatedTanks));
    
    int calculatedHealers = static_cast<int>(std::ceil(raidSize * HEALER_RATIO));
    comp.healers = std::max(MIN_HEALERS, std::min(MAX_HEALERS, calculatedHealers));
    
    comp.dps = raidSize - comp.tanks - comp.healers;
    
    if (comp.tanks + comp.healers + comp.dps > raidSize)
    {
        comp.dps = raidSize - comp.tanks - comp.healers;
    }
    
    return comp;
}

std::vector<std::string> RaidBuilderCommand::HandleRaidBuilderCommand(ChatHandler* handler, char const* args)
{
    std::vector<std::string> messages;
    Player* master = handler->GetSession()->GetPlayer();
    
    if (!master)
    {
        messages.push_back("Error: No player found");
        return messages;
    }

    std::string argStr = args ? args : "";
    std::istringstream iss(argStr);
    std::string command;
    int raidSize = 0;

    iss >> command;
    if (iss >> raidSize)
    {
    }
    else
    {
        if (!command.empty() && std::all_of(command.begin(), command.end(), ::isdigit))
        {
            raidSize = std::stoi(command);
        }
    }

    if (command == "clear")
    {
        Group* group = master->GetGroup();
        if (group)
        {
            group->Disband();
            messages.push_back("Group cleared");
        }
        else
        {
            messages.push_back("No group to clear");
        }
        return messages;
    }

    if (raidSize < RaidComposition::MIN_RAID_SIZE || raidSize > RaidComposition::MAX_RAID_SIZE)
    {
        messages.push_back("Error: Invalid raid size. Use " + std::to_string(RaidComposition::MIN_RAID_SIZE) + "-" + std::to_string(RaidComposition::MAX_RAID_SIZE) + " players");
        messages.push_back("Usage: .playerbots raid [" + std::to_string(RaidComposition::MIN_RAID_SIZE) + "-" + std::to_string(RaidComposition::MAX_RAID_SIZE) + "]");
        return messages;
    }

    if (sPlayerbotAIConfig->addClassCommand == 0)
    {
        messages.push_back("Error: You do not have permission to create bots");
        return messages;
    }

    RaidComposition::RoleComposition comp = RaidComposition::GetOptimalComposition(raidSize);
    
    std::string masterRole = GetPlayerRole(master);
    bool masterIsTank = (masterRole == "tank");
    if (masterRole == "tank" && comp.tanks > 0) comp.tanks--;
    else if (masterRole == "heal" && comp.healers > 0) comp.healers--;
    else if (masterRole == "dps") comp.dps--;
    
    Group* existingGroup = master->GetGroup();
    if (existingGroup)
    {
        int currentMembers = existingGroup->GetMembersCount();
        if (currentMembers >= raidSize)
        {
            messages.push_back("You already have " + std::to_string(currentMembers) + " members (>= " + std::to_string(raidSize) + ")");
            messages.push_back("Use .playerbots raid " + std::to_string(currentMembers + 5) + " to expand");
            return messages;
        }
        messages.push_back("Expanding from " + std::to_string(currentMembers) + " to " + std::to_string(raidSize) + " members");
    }
    else
    {
        messages.push_back("Creating " + std::to_string(raidSize) + "-man raid...");
    }

    std::vector<Player*> availableBots;
    std::map<uint8, std::vector<Player*>> botsByClass;
    
    for (auto it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* bot = it->second;
        if (bot && bot->IsInWorld() && bot->GetTeamId() == master->GetTeamId() && bot->GetLevel() == master->GetLevel())
        {
            if (bot->GetGroup())
            {
                bot->GetGroup()->RemoveMember(bot->GetGUID());
            }
            availableBots.push_back(bot);
            botsByClass[bot->getClass()].push_back(bot);
        }
    }
    
    int neededBots = 0;
    if (existingGroup)
    {
        int currentMembers = existingGroup->GetMembersCount();
        if (currentMembers >= raidSize)
        {
            messages.push_back("You already have " + std::to_string(currentMembers) + " members (>= " + std::to_string(raidSize) + ")");
            messages.push_back("Use .playerbots raid clear to start fresh, or .playerbots raid " + std::to_string(currentMembers + 5) + " to expand");
            return messages;
        }
        neededBots = raidSize - currentMembers;
        messages.push_back("Expanding to " + std::to_string(raidSize) + " members");
    }
    else
    {
        neededBots = raidSize - 1;
        messages.push_back("Creating " + std::to_string(raidSize) + "-man raid");
    }
    
    if (availableBots.size() < neededBots)
    {
        messages.push_back("Found " + std::to_string(availableBots.size()) + "/" + std::to_string(neededBots) + " bots");
    }

    std::vector<Player*> selectedBots;
    std::vector<Player*> tankBots;
    int selectedTanks = 0, selectedHealers = 0, selectedDps = 0;
    
    std::map<uint8, std::vector<std::string>> classRoles = {
        {CLASS_WARRIOR, {"tank", "dps"}},
        {CLASS_PALADIN, {"tank", "heal", "dps"}},
        {CLASS_HUNTER, {"dps"}},
        {CLASS_ROGUE, {"dps"}},
        {CLASS_PRIEST, {"heal", "dps"}},
        {CLASS_DEATH_KNIGHT, {"tank", "dps"}},
        {CLASS_SHAMAN, {"heal", "dps"}},
        {CLASS_MAGE, {"dps"}},
        {CLASS_WARLOCK, {"dps"}},
        {CLASS_DRUID, {"tank", "heal", "dps"}}
    };
    
    std::vector<uint8> tankClasses = {CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT, CLASS_DRUID};
    for (uint8 classId : tankClasses)
    {
        if (selectedTanks >= comp.tanks) break;
        if (!botsByClass[classId].empty())
        {
            for (auto it = botsByClass[classId].begin(); it != botsByClass[classId].end(); ++it)
            {
                Player* bot = *it;
                if (GetPlayerRole(bot) == "tank")
                {
                    botsByClass[classId].erase(it);
                    selectedBots.push_back(bot);
                    tankBots.push_back(bot);
                    selectedTanks++;
                    break;
                }
            }
        }
    }
    
    std::vector<uint8> healerClasses = {CLASS_PRIEST, CLASS_PALADIN, CLASS_SHAMAN, CLASS_DRUID};
    for (uint8 classId : healerClasses)
    {
        if (selectedHealers >= comp.healers) break;
        if (!botsByClass[classId].empty())
        {
            for (auto it = botsByClass[classId].begin(); it != botsByClass[classId].end(); ++it)
            {
                Player* bot = *it;
                if (GetPlayerRole(bot) == "heal")
                {
                    botsByClass[classId].erase(it);
                    selectedBots.push_back(bot);
                    selectedHealers++;
                    break;
                }
            }
        }
    }
    
    int maxBotsToAdd = std::min(static_cast<int>(availableBots.size()), neededBots);
    
    for (int i = 0; i < maxBotsToAdd && selectedBots.size() < neededBots; ++i)
    {
        Player* bot = availableBots[i];
        if (std::find(selectedBots.begin(), selectedBots.end(), bot) == selectedBots.end())
        {
            selectedBots.push_back(bot);
            
            std::string role = GetPlayerRole(bot);
            
            if (selectedTanks < comp.tanks && role == "tank")
            {
                tankBots.push_back(bot);
                selectedTanks++;
            }
            else if (selectedHealers < comp.healers && role == "heal")
            {
                selectedHealers++;
            }
            else
            {
                selectedDps++;
            }
        }
    }
    

    messages.push_back("Raid created with " + std::to_string(selectedBots.size() + 1) + " members");
    
    
    if (selectedBots.size() > 0)
    {
        for (Player* bot : selectedBots)
        {
            AddBotToGroup(master, bot);
        }
        
        Group* finalGroup = master->GetGroup();
        if (finalGroup)
        {
            if (finalGroup->GetMembersCount() >= RaidComposition::GROUP_TO_RAID_THRESHOLD && !finalGroup->isRaidGroup())
            {
                finalGroup->ConvertToRaid();
            }
            
            if (masterIsTank)
            {
                tankBots.insert(tankBots.begin(), master);
            }
            
            if (!tankBots.empty())
            {
                AssignTankMarkers(finalGroup, tankBots);
            }
        }
    }
    
    if (selectedBots.size() < neededBots)
    {
        messages.push_back("Not enough bots available. Try again later.");
    }
    
    return messages;
}

uint8 RaidBuilderCommand::GetClassIdFromName(const std::string& className)
{
    if (className == "warrior") return CLASS_WARRIOR;
    if (className == "paladin") return CLASS_PALADIN;
    if (className == "hunter") return CLASS_HUNTER;
    if (className == "rogue") return CLASS_ROGUE;
    if (className == "priest") return CLASS_PRIEST;
    if (className == "dk") return CLASS_DEATH_KNIGHT;
    if (className == "shaman") return CLASS_SHAMAN;
    if (className == "mage") return CLASS_MAGE;
    if (className == "warlock") return CLASS_WARLOCK;
    if (className == "druid") return CLASS_DRUID;
    return 0;
}

std::string RaidBuilderCommand::GetClassNameFromId(uint8 classId)
{
    switch (classId)
    {
        case CLASS_WARRIOR: return "warrior";
        case CLASS_PALADIN: return "paladin";
        case CLASS_HUNTER: return "hunter";
        case CLASS_ROGUE: return "rogue";
        case CLASS_PRIEST: return "priest";
        case CLASS_DEATH_KNIGHT: return "dk";
        case CLASS_SHAMAN: return "shaman";
        case CLASS_MAGE: return "mage";
        case CLASS_WARLOCK: return "warlock";
        case CLASS_DRUID: return "druid";
        default: return "unknown";
    }
}

std::string RaidBuilderCommand::GetPlayerRole(Player* player)
{
    if (!player) return "dps";
    
    uint8 playerClass = player->getClass();
    uint8 spec = AiFactory::GetPlayerSpecTab(player);
    
    switch (playerClass)
    {
        case CLASS_WARRIOR:
            if (spec == RaidComposition::SPEC_PROTECTION) return "tank";
            return "dps";
            
        case CLASS_PALADIN:
            if (spec == RaidComposition::SPEC_PROTECTION) return "tank";
            if (spec == RaidComposition::SPEC_HOLY) return "heal";
            return "dps";
            
        case CLASS_HUNTER:
            return "dps";
            
        case CLASS_ROGUE:
            return "dps";
            
        case CLASS_PRIEST:
            if (spec == RaidComposition::SPEC_HOLY) return "heal";
            return "dps";
            
        case CLASS_DEATH_KNIGHT:
            if (spec == RaidComposition::SPEC_BLOOD) return "tank";
            return "dps";
            
        case CLASS_SHAMAN:
            if (spec == RaidComposition::SPEC_RESTORATION) return "heal";
            return "dps";
            
        case CLASS_MAGE:
            return "dps";
            
        case CLASS_WARLOCK:
            return "dps";
            
        case CLASS_DRUID:
            if (spec == RaidComposition::SPEC_BALANCE) return "dps";
            if (spec == RaidComposition::SPEC_FERAL) return "tank";
            if (spec == RaidComposition::SPEC_RESTORATION) return "heal";
            return "dps";
            
        default:
            return "dps";
    }
}

bool RaidBuilderCommand::AddBotToGroup(Player* master, Player* bot)
{
    if (!master || !bot) return false;
    
    try
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            group = new Group();
            group->Create(master);
        }
        
        if (group->GetMembersCount() >= RaidComposition::GROUP_TO_RAID_THRESHOLD && !group->isRaidGroup())
        {
            group->ConvertToRaid();
        }
        
        if (group->AddMember(bot))
        {
            if (GET_PLAYERBOT_AI(bot))
            {
                GET_PLAYERBOT_AI(bot)->SetMaster(master);
            }
            
            if (bot->GetGroup() == group)
            {
                return true;
            }
            else
            {
                group->RemoveMember(bot->GetGUID());
                return false;
            }
        }
    }
    catch (const std::exception& e)
    {
    }
    
    return false;
}


void RaidBuilderCommand::AssignTankMarkers(Group* group, const std::vector<Player*>& tanks)
{
    if (!group || tanks.empty())
        return;

    const std::vector<uint8> tankIcons = {
        RaidComposition::RAID_ICON_CROSS,
        RaidComposition::RAID_ICON_STAR,
        RaidComposition::RAID_ICON_DIAMOND,
        RaidComposition::RAID_ICON_TRIANGLE
    };
    int iconIndex = 0;
    for (auto tank : tanks)
    {
        if (iconIndex >= static_cast<int>(tankIcons.size()))
            break;
            
        if (tank && tank->IsInWorld())
        {
            Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
            if (leader)
            {
                group->SetTargetIcon(tankIcons[iconIndex], leader->GetGUID(), tank->GetGUID());
                iconIndex++;
            }
        }
    }
}