// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "GameCommands.h"
#include "GamePlayer.h"
#include "buildings/nobShipYard.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHarborBuilding.h"
#include "nodeObjs/noShip.h"
#include "world/GameWorldGame.h"
#include <iostream>

namespace gc{

    void SetFlag::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.SetFlag(pt_, playerId);
    }

    void DestroyFlag::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        gwg.DestroyFlag(pt_);
    }

    void BuildRoad::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.BuildRoad(playerId, boat_road, pt_, route);
    }

    void DestroyRoad::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        gwg.DestroyRoad(pt_, start_dir);
    }

    void UpgradeRoad::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        gwg.UpgradeRoad(pt_, start_dir);
    }

    void ChangeDistribution::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.ChangeDistribution(data);
    }

    void ChangeBuildOrder::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.ChangeBuildOrder(order_type, data);
    }

    void SetBuildingSite::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.SetBuildingSite(bt, pt_, playerId);
    }

    void DestroyBuilding::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.DestroyBuilding(pt_, playerId);
    }

    void ChangeTransport::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.ConvertTransportData(data);
    }

    void ChangeMilitary::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.ChangeMilitarySettings(data);
    }

    void ChangeTools::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.ChangeToolsSettings(data, orders);
    }

    void CallGeologist::Execute(GameWorldGame& gwg, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        if(gwg.GetNO(pt_)->GetGOT() == GOT_FLAG)
            player.CallFlagWorker(pt_, JOB_GEOLOGIST);
    }

    void CallScout::Execute(GameWorldGame& gwg, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        if(gwg.GetNO(pt_)->GetGOT() == GOT_FLAG)
            player.CallFlagWorker(pt_, JOB_SCOUT);
    }

    void Attack::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.Attack(playerId, pt_, soldiers_count, strong_soldiers);
    }

    void SeaAttack::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.AttackViaSea(playerId, pt_, soldiers_count, strong_soldiers);
    }

    void SetCoinsAllowed::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        nobMilitary* const bld = gwg.GetSpecObj<nobMilitary>(pt_);
        if(bld && bld->GetPlayer() == playerId)
            bld->SetCoinsAllowed(enabled);
    }

    void SendSoldiersHome::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        nobMilitary* const bld = gwg.GetSpecObj<nobMilitary>(pt_);
        if(bld && bld->GetPlayer() == playerId)
            bld->SendSoldiersHome();
    }

    void OrderNewSoldiers::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        nobMilitary* const bld = gwg.GetSpecObj<nobMilitary>(pt_);
        if(bld && bld->GetPlayer() == playerId)
            bld->OrderNewSoldiers();
    }

    void SetProductionEnabled::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobUsual* const bld = gwg.GetSpecObj<nobUsual>(pt_);
        if(bld)
            bld->SetProductionEnabled(enabled);
    }

    void SetInventorySetting::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobBaseWarehouse* const bld = gwg.GetSpecObj<nobBaseWarehouse>(pt_);
        if(bld)
            bld->SetInventorySetting(isJob, type, state);
    }

    void SetAllInventorySettings::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobBaseWarehouse* const bld = gwg.GetSpecObj<nobBaseWarehouse>(pt_);
        if(bld)
            bld->SetAllInventorySettings(isJob, states);
    }

    void ChangeReserve::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobBaseWarehouse* const bld = gwg.GetSpecObj<nobBaseWarehouse>(pt_);
        if(bld)
            bld->SetRealReserve(rank, count);
    }

    void Surrender::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.Surrender();
    }

    void CheatArmageddon::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        gwg.Armageddon();
    }

    void DestroyAll::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char playerId)
    {
        gwg.Armageddon(playerId);
    }

    void SuggestPact::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.SuggestPact(targetPlayer, pt, duration);
    }

    void AcceptPact::Execute(GameWorldGame& gwg, GamePlayer&  player, const unsigned char playerId)
    {
        gwg.GetPlayer(fromPlayer).AcceptPact(id, pt, playerId);
    }

    void CancelPact::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        player.CancelPact(pt, this->player);
    }

    void NotifyAlliesOfLocation::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char playerId)
    {
	    player.NotifyAlliesOfLocation(pt_);
    }

    void ToggleShipYardMode::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobShipYard* const bld = gwg.GetSpecObj<nobShipYard>(pt_);
        if(bld)
            bld->ToggleMode();
    }

    void StartExpedition::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobHarborBuilding* const bld = gwg.GetSpecObj<nobHarborBuilding>(pt_);
        if(bld)
            bld->StartExpedition();
    }

    void StartExplorationExpedition::Execute(GameWorldGame& gwg, GamePlayer&  /*player*/, const unsigned char  /*playerId*/)
    {
        nobHarborBuilding* const bld = gwg.GetSpecObj<nobHarborBuilding>(pt_);
        if(bld)
            bld->StartExplorationExpedition();
    }

    void ExpeditionCommand::Execute(GameWorldGame&  /*gwg*/, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        noShip* ship = player.GetShipByID(this->ship_id);
        if(!ship)
            return;

        if(this->action == FOUNDCOLONY)
            ship->FoundColony();
        else if(this->action == CANCELEXPEDITION)
            ship->CancelExpedition();
        else
            ship->ContinueExpedition(action - 2);
    }

    /// Fuehrt das GameCommand aus
    void TradeOverLand::Execute(GameWorldGame& gwg, GamePlayer& player, const unsigned char  /*playerId*/)
    {
        nobBaseWarehouse* const bld = gwg.GetSpecObj<nobBaseWarehouse>(pt_);
        if(bld)
            player.Trade(bld, gt, job, count);
    }

}
 // namespace gc
