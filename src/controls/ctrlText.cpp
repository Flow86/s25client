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
#include "ctrlText.h"
#include "ogl/glArchivItem_Font.h"

ctrlBaseText::ctrlBaseText(const std::string& text, const unsigned color, glArchivItem_Font* font) :
    text(text), color_(color), font(font)
{
}

ctrlText::ctrlText(Window* parent,
                   unsigned int id,
                   unsigned short x,
                   unsigned short y,
                   const std::string& text,
                   unsigned int color,
                   unsigned int format,
                   glArchivItem_Font* font)
    : Window(x, y, id, parent), ctrlBaseText(text, color, font), format(format)
{
}

/**
 *  zeichnet das Fenster.
 */
bool ctrlText::Draw_()
{
    if(!text.empty())
        font->Draw(GetDrawPos(), text, format, color_);

    return true;
}
