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
#include <build_version.h>
#include "BinaryFile.h"
#include "SavedFile.h"
#include "BasePlayerInfo.h"
#include "helpers/Deleter.h"
#include "libutil/src/Serializer.h"
#include "libutil/src/Log.h"
#include <cstring>

SavedFile::SavedFile() : save_time(0)
{
}

SavedFile::~SavedFile()
{}

void SavedFile::WriteVersion(BinaryFile& file, unsigned int signature_length, const char* signature, unsigned short version)
{
    // Signatur schreiben
    file.WriteRawData(signature, signature_length);

    // Version vom Programm reinschreiben (mal 0 mit reinschreiben, damits ne runde 8 ergibt!)
    file.WriteRawData(GetWindowRevision(), 8);

    // Version des Save-Formats
    file.WriteUnsignedShort(version);
}

bool SavedFile::ValidateFile(BinaryFile& file, unsigned int signature_length, const char* signature, unsigned short version)
{
    char read_signature[32];

    file.ReadRawData(read_signature, signature_length);

    // Signatur überprüfen
    if(memcmp(read_signature, signature, signature_length) != 0)
    {
        // unterscheiden sich! --> raus
        LOG.lprintf("Error: File is not in a valid format! File path: %s\n", file.getFilePath().c_str());
        return false;
    }

    // Programmversion überspringen
    file.Seek(8, SEEK_CUR);

    // Version überprüfen
    unsigned short read_version = file.ReadUnsignedShort();
    if(read_version != version)
    {
        // anderes Dateiformat --> raus
        LOG.lprintf("Warning: File has an old version and cannot be used (version: %u; expected: %u, file path: %s)!\n", read_version, version, file.getFilePath().c_str());
        return false;
    }

    return true;
}

void SavedFile::WritePlayerData(BinaryFile& file)
{
    Serializer ser;
    ser.PushUnsignedChar(players.size());
    for(std::vector<BasePlayerInfo>::const_iterator it = players.begin(); it != players.end(); ++it)
        it->Serialize(ser, true);

    ser.WriteToFile(file);
}

void SavedFile::ReadPlayerData(BinaryFile& file)
{
    players.clear();
    Serializer ser;
    ser.ReadFromFile(file);
    const unsigned playerCt = ser.PopUnsignedChar();
    players.reserve(playerCt);
    for(unsigned i = 0; i < playerCt; i++)
        players.push_back(BasePlayerInfo(ser, true));
}

/**
 *  schreibt die GlobalGameSettings in die Datei.
 */
void SavedFile::WriteGGS(BinaryFile& file)
{
    Serializer ser;
    ggs.Serialize(ser);
    ser.WriteToFile(file);
}

/**
 *  liest die GlobalGameSettings aus der Datei.
 */
void SavedFile::ReadGGS(BinaryFile& file)
{
    Serializer ser;
    ser.ReadFromFile(file);
    ggs.Deserialize(ser);
}

const BasePlayerInfo& SavedFile::GetPlayer(unsigned idx) const
{
    return players[idx];
}

unsigned SavedFile::GetPlayerCount()
{
    return players.size();
}

void SavedFile::AddPlayer(const BasePlayerInfo& player)
{
    players.push_back(player);
}

void SavedFile::ClearPlayers()
{
    players.clear();
}
