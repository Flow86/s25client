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
#include "buildings/noBuildingSite.h"
#include "GamePlayer.h"
#include "Ware.h"
#include "figures/nofBuilder.h"
#include "figures/nofPlaner.h"
#include "SerializedGameData.h"
#include "gameData/BuildingConsts.h"
#include "FOWObjects.h"
#include "ogl/glSmartBitmap.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "world/GameWorldGame.h"
#include "Loader.h"
#include "helpers/converters.h"
#include "libutil/src/colors.h"
#include <stdexcept>
class noRoadNode;

noBuildingSite::noBuildingSite(const BuildingType type, const MapPoint pos, const unsigned char player)
    : noBaseBuilding(NOP_BUILDINGSITE, type, pos, player), state(STATE_BUILDING), planer(NULL), builder(NULL), boards(0), stones(0), used_boards(0), used_stones(0), build_progress(0)
{
    // Überprüfen, ob die Baustelle erst noch planiert werden muss (nur bei mittleren/großen Gebäuden)
    if(GetSize() == BQ_HOUSE || GetSize() == BQ_CASTLE || GetSize() == BQ_HARBOR)
    {
        // Höhe auf dem Punkt, wo die Baustelle steht
        int altitude = gwg->GetNode(pos).altitude;

        for(unsigned i = 0; i < 6; ++i)
        {
            // Richtung 4 wird nicht planiert (Flagge)
            if(i != 4)
            {
                // Gibt es da Differenzen?
                if(altitude - gwg->GetNeighbourNode(pos, i).altitude != 0)
                    state = STATE_PLANING;
            }
        }
    }

    // Wir hätten gerne einen Planierer/Bauarbeiter...
    gwg->GetPlayer(player).AddJobWanted((state == STATE_PLANING) ? JOB_PLANER : JOB_BUILDER, this);

    // Bauwaren anfordern
    OrderConstructionMaterial();

    // Baustelle in den Index eintragen, damit die Wirtschaft auch Bescheid weiß
    gwg->GetPlayer(player).AddBuildingSite(this);
}

/// Konstruktor für Hafenbaustellen vom Schiff aus
noBuildingSite::noBuildingSite(const MapPoint pos, const unsigned char player)
    : noBaseBuilding(NOP_BUILDINGSITE, BLD_HARBORBUILDING, pos, player),
      state(STATE_BUILDING),
      planer(NULL),
      boards(BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards),
      stones(BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones),
      used_boards(0),
      used_stones(0),
      build_progress(0)
{
    builder = new nofBuilder(pos, player, this);
    GamePlayer& owner = gwg->GetPlayer(player);
    // Baustelle in den Index eintragen, damit die Wirtschaft auch Bescheid weiß
    owner.AddBuildingSite(this);
    // Bauarbeiter auch auf der Karte auftragen
    gwg->AddFigure(builder, pos);

    // Baumaterialien in der Inventur verbuchen
    owner.DecreaseInventoryWare(GD_BOARDS, boards);
    owner.DecreaseInventoryWare(GD_STONES, stones);
}

noBuildingSite::~noBuildingSite()
{
}

void noBuildingSite::Destroy_noBuildingSite()
{
    // Bauarbeiter/Planierer Bescheid sagen
    if(builder)
    {
        builder->LostWork();
        builder = NULL;
    }else if(planer)
    {
        planer->LostWork();
        planer = NULL;
    }else
        gwg->GetPlayer(player).JobNotWanted(this);

    RTTR_Assert(!builder);
    RTTR_Assert(!planer);

    // Bestellte Waren Bescheid sagen
    for(std::list<Ware*>::iterator it = ordered_boards.begin(); it != ordered_boards.end(); ++it)
        WareNotNeeded(*it);
    ordered_boards.clear();
    for(std::list<Ware*>::iterator it = ordered_stones.begin(); it != ordered_stones.end(); ++it)
        WareNotNeeded(*it);
    ordered_stones.clear();

    // und Feld wird leer
    gwg->SetNO(pos, NULL);

    // Baustelle wieder aus der Liste entfernen - dont forget about expedition harbor status
    bool expeditionharbor = IsHarborBuildingSiteFromSea();
    gwg->GetPlayer(player).RemoveBuildingSite(this);

    Destroy_noBaseBuilding();

    // Hafenbaustelle?
    if(expeditionharbor)
    {
        // Land neu berechnen nach zerstören weil da schon straßen etc entfernt werden
        gwg->RecalcTerritory(*this, true, false);
    }
    gwg->RecalcBQAroundPointBig(pos);
}

void noBuildingSite::Serialize_noBuildingSite(SerializedGameData& sgd) const
{
    Serialize_noBaseBuilding(sgd);

    sgd.PushUnsignedChar(static_cast<unsigned char>(state));
    sgd.PushObject(planer, true);
    sgd.PushObject(builder, true);
    sgd.PushUnsignedChar(boards);
    sgd.PushUnsignedChar(stones);
    sgd.PushUnsignedChar(used_boards);
    sgd.PushUnsignedChar(used_stones);
    sgd.PushUnsignedChar(build_progress);
    sgd.PushObjectContainer(ordered_boards, true);
    sgd.PushObjectContainer(ordered_stones, true);
}

noBuildingSite::noBuildingSite(SerializedGameData& sgd, const unsigned obj_id) : noBaseBuilding(sgd, obj_id),
    state(static_cast<State>(sgd.PopUnsignedChar())),
    planer(sgd.PopObject<nofPlaner>(GOT_NOF_PLANER)),
    builder(sgd.PopObject<nofBuilder>(GOT_NOF_BUILDER)),
    boards(sgd.PopUnsignedChar()),
    stones(sgd.PopUnsignedChar()),
    used_boards(sgd.PopUnsignedChar()),
    used_stones(sgd.PopUnsignedChar()),
    build_progress(sgd.PopUnsignedChar())
{
    sgd.PopObjectContainer(ordered_boards, GOT_WARE);
    sgd.PopObjectContainer(ordered_stones, GOT_WARE);
}

void noBuildingSite::OrderConstructionMaterial()
{
    // Bei Planieren keine Waren bestellen
    if(state == STATE_PLANING)
        return;

    // Bretter
    GamePlayer& owner = gwg->GetPlayer(player);
    for(int i = used_boards + boards + ordered_boards.size(); i < BUILDING_COSTS[owner.nation][type_].boards; ++i)
    {
        Ware* w = owner.OrderWare(GD_BOARDS, this);
        if(!w)
            break;
        RTTR_Assert(helpers::contains(ordered_boards, w));
    }
    // Steine
    for(int i = used_stones + stones + ordered_stones.size(); i < BUILDING_COSTS[owner.nation][type_].stones; ++i)
    {
        Ware* w = owner.OrderWare(GD_STONES, this);
        if(!w)
            break;
        RTTR_Assert(helpers::contains(ordered_stones, w));
    }
}

void noBuildingSite::Draw(DrawPoint drawPt)
{
    if(state == STATE_PLANING)
    {
        // Baustellenschild mit Schatten zeichnen
        LOADER.GetNationImage(gwg->GetPlayer(player).nation, 450)->Draw(drawPt);
        LOADER.GetNationImage(gwg->GetPlayer(player).nation, 451)->Draw(drawPt, 0, 0, 0, 0, 0, 0, COLOR_SHADOW);
    }
    else
    {
        // Baustellenstein und -schatten zeichnen
        LOADER.GetNationImage(gwg->GetPlayer(player).nation, 455)->Draw(drawPt);
        LOADER.GetNationImage(gwg->GetPlayer(player).nation, 456)->Draw(drawPt, 0, 0, 0, 0, 0, 0, COLOR_SHADOW);


        // Waren auf der Baustelle

        // Bretter
        DrawPoint doorPos = drawPt + DrawPoint(GetDoorPointX(), GetDoorPointY());
        for(unsigned char i = 0; i < boards; ++i)
            LOADER.GetMapImageN(2200 + GD_BOARDS)->Draw(doorPos - DrawPoint(5, 10 + i * 4));
        // Steine
        for(unsigned char i = 0; i < stones; ++i)
            LOADER.GetMapImageN(2200 + GD_STONES)->Draw(doorPos + DrawPoint(8, -12 - i * 4));

        // bis dahin gebautes Haus zeichnen

        // Rohbau

        // ausrechnen, wie weit er ist
        unsigned int p1 = 0, p2 = 0;

        if(BUILDING_COSTS[nation][GetBuildingType()].stones)
        {
            // Haus besteht aus Steinen und Brettern
            p1 = min<unsigned int>(build_progress, BUILDING_COSTS[nation][GetBuildingType()].boards * 8);
            p2 = BUILDING_COSTS[nation][GetBuildingType()].boards * 8;
        }
        else
        {
            // Haus besteht nur aus Brettern, dann 50:50
            p1 = min<unsigned int>(build_progress, BUILDING_COSTS[nation][GetBuildingType()].boards * 4);
            p2 = BUILDING_COSTS[nation][GetBuildingType()].boards * 4;
        }

        LOADER.building_cache[nation][type_][1].drawPercent(drawPt, p1 * 100 / p2);

        // Das richtige Haus
        if(BUILDING_COSTS[nation][GetBuildingType()].stones)
        {
            // Haus besteht aus Steinen und Brettern
            p1 = ((build_progress >  BUILDING_COSTS[nation][GetBuildingType()].boards * 8) ? (build_progress - BUILDING_COSTS[nation][GetBuildingType()].boards * 8) : 0);
            p2 = BUILDING_COSTS[nation][GetBuildingType()].stones * 8;
        }
        else
        {
            // Haus besteht nur aus Brettern, dann 50:50
            p1 = ((build_progress >  BUILDING_COSTS[nation][GetBuildingType()].boards * 4) ? (build_progress - BUILDING_COSTS[nation][GetBuildingType()].boards * 4) : 0);
            p2 = BUILDING_COSTS[nation][GetBuildingType()].boards * 4;
        }

        LOADER.building_cache[nation][type_][0].drawPercent(drawPt, p1 * 100 / p2);
    }

    //char number[256];
    //sprintf(number,"%u",obj_id);
    //NormalFont->Draw(x,y,number,0,0xFFFF0000);
}

/// Erzeugt von ihnen selbst ein FOW Objekt als visuelle "Erinnerung" für den Fog of War
FOWObject* noBuildingSite::CreateFOWObject() const
{
    return new fowBuildingSite(state == STATE_PLANING, type_, nation, build_progress);
}


void noBuildingSite::GotWorker(Job  /*job*/, noFigure* worker)
{
    // Aha, wir haben nen Planierer/Bauarbeiter bekommen
    if(state == STATE_PLANING)
    {
        RTTR_Assert(worker->GetGOT() == GOT_NOF_PLANER);
        planer = static_cast<nofPlaner*>(worker);
    }
    else
    {
        RTTR_Assert(worker->GetGOT() == GOT_NOF_BUILDER);
        builder = static_cast<nofBuilder*>(worker);
    }
}

void noBuildingSite::Abrogate()
{
    planer = NULL;
    builder = NULL;

    gwg->GetPlayer(player).AddJobWanted((state == STATE_PLANING) ? JOB_PLANER : JOB_BUILDER, this);
}

unsigned noBuildingSite::CalcDistributionPoints(noRoadNode*  /*start*/, const GoodType goodtype)
{
    // Beim Planieren brauchen wir noch gar nichts
    if(state == STATE_PLANING)
        return 0;

    // We only need boards and stones.
    if(goodtype != GD_BOARDS && goodtype != GD_STONES)
        return 0;

    const unsigned curBoards = ordered_boards.size() + boards + used_boards;
    const unsigned curStones = ordered_stones.size() + stones + used_stones;
    RTTR_Assert(curBoards <= BUILDING_COSTS[nation][this->type_].boards);
    RTTR_Assert(curStones <= BUILDING_COSTS[nation][this->type_].stones);

    // Wenn wir schon genug Baumaterial haben, brauchen wir nichts mehr
    if((goodtype == GD_BOARDS && curBoards == BUILDING_COSTS[nation][this->type_].boards) ||
            (goodtype == GD_STONES && curStones == BUILDING_COSTS[nation][this->type_].stones))
        return 0;

    // 10000 als Basis wählen, damit man auch noch was abziehen kann
    BOOST_CONSTEXPR_OR_CONST unsigned basePoints = 10000;
    unsigned points = basePoints;

    // Baumaterial mit einberechnen (wer noch am wenigsten braucht, soll mehr Punkte kriegen, da ja möglichst
    // zuerst Gebäude fertiggestellt werden sollten)
    points -= (BUILDING_COSTS[nation][type_].boards - curBoards) * 20;
    points -= (BUILDING_COSTS[nation][type_].stones - curStones) * 20;

    // Baupriorität mit einberechnen (niedriger = höhere Priorität, daher - !)
    const unsigned buildingSitePrio = gwg->GetPlayer(player).GetBuidingSitePriority(this) * 30;

    if(points > buildingSitePrio)
        points -= buildingSitePrio;
    else
        points = 0;

    RTTR_Assert(points <= basePoints); // Underflow detection. Should never happen...

    return points;
}

void noBuildingSite::AddWare(Ware*& ware)
{
    RTTR_Assert(state == STATE_BUILDING);

    if(ware->type == GD_BOARDS)
    {
        RTTR_Assert(helpers::contains(ordered_boards, ware));
        ordered_boards.remove(ware);
        ++boards;
    }
    else if(ware->type == GD_STONES)
    {
        RTTR_Assert(helpers::contains(ordered_stones, ware));
        ordered_stones.remove(ware);
        ++stones;
    }
    else
        throw std::logic_error("Wrong ware type " + helpers::toString(ware->type));

    // Inventur entsprechend verringern
    gwg->GetPlayer(player).DecreaseInventoryWare(ware->type, 1);
    gwg->GetPlayer(player).RemoveWare(ware);
    deletePtr(ware);
}

void noBuildingSite::WareLost(Ware* ware)
{
    RTTR_Assert(state == STATE_BUILDING);

    if(ware->type == GD_BOARDS)
    {
        RTTR_Assert(helpers::contains(ordered_boards, ware));
        ordered_boards.remove(ware);
    }
    else if(ware->type == GD_STONES)
    {
        RTTR_Assert(helpers::contains(ordered_stones, ware));
        ordered_stones.remove(ware);
    }
    else
        throw std::logic_error("Wrong ware type lost " + helpers::toString(ware->type));

    OrderConstructionMaterial();
}


void noBuildingSite::TakeWare(Ware* ware)
{
    RTTR_Assert(state == STATE_BUILDING);

    // Ware in die Bestellliste aufnehmen
    if(ware->type == GD_BOARDS)
    {
        RTTR_Assert(!helpers::contains(ordered_boards, ware));
        ordered_boards.push_back(ware);
    }else if(ware->type == GD_STONES)
    {
        RTTR_Assert(!helpers::contains(ordered_stones, ware));
        ordered_stones.push_back(ware);
    }else
        throw std::logic_error("Wrong ware type " + helpers::toString(ware->type));
}


bool noBuildingSite::IsBuildingComplete()
{
    return (build_progress == BUILDING_COSTS[nation][type_].boards * 8 + BUILDING_COSTS[nation][type_].stones * 8);
}

unsigned char noBuildingSite::GetBuildProgress(bool percent) const
{
    if(!percent)
        return build_progress;

    unsigned costs = BUILDING_COSTS[nation][type_].boards * 8 + BUILDING_COSTS[nation][type_].stones * 8;
    unsigned progress = (((unsigned) build_progress) * 100) / costs;

    return (unsigned char)progress;
}

/// Aufgerufen, wenn Planierung beendet wurde
void noBuildingSite::PlaningFinished()
{
    /// Normale Baustelle
    state = STATE_BUILDING;
    planer = 0;

    // Wir hätten gerne einen Bauarbeiter...
    gwg->GetPlayer(player).AddJobWanted(JOB_BUILDER, this);

    // Bauwaren anfordern
    OrderConstructionMaterial();
}

/// Gibt zurück, ob eine bestimmte Baustellen eine Baustelle ist, die vom Schiff aus errichtet wurde
bool noBuildingSite::IsHarborBuildingSiteFromSea() const
{
    if(this->type_ == BLD_HARBORBUILDING)
        return gwg->IsHarborBuildingSiteFromSea(this);
    else
        return false;
}
