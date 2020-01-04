///////////////////////////////////////////////////////////////////////////////
// Copyright 2019 Alan Chambers (unicycle.bloke@gmail.com)
//
// This file is part of yagl.
//
// yagl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// yagl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with yagl. If not, see <https://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////
#include "SpriteIndexRecord.h"
#include "RealSpriteRecord.h"
#include "RecolourRecord.h"
#include "StreamHelpers.h"
#include "ActionFFRecord.h"
#include "CommandLineOptions.h"


void SpriteIndexRecord::read(std::istream& is, const GRFInfo& info)
{
    m_sprite_id = read_uint32(is);
}


void SpriteIndexRecord::write(std::ostream& os, const GRFInfo& info) const
{
    write_uint32(os, m_sprite_id);
}  


static constexpr const char* str_sprite_id = "sprite_id";


void SpriteIndexRecord::print(std::ostream& os, const SpriteZoomMap& sprites, uint16_t indent) const
{
    os << pad(indent) << str_sprite_id << ": " << to_hex(m_sprite_id) << "\n";
    os << pad(indent) << "{" << '\n';

    if (sprites.find(m_sprite_id) != sprites.end())
    {
        const SpriteZoomVector& sprite_list = sprites.at(m_sprite_id);
        for (auto sprite: sprite_list)
        {
            sprite->print(os, sprites, indent + 4);
        }
    }
    else
    {
        if (CommandLineOptions::options().debug())
        {
            std::cout << pad(indent + 4) << "Missing sprites.\n";
        }
    }

    os << pad(indent) << "}" << '\n';
}


void SpriteIndexRecord::parse(TokenStream& is)
{
    is.match_ident(str_sprite_id);
    is.match(TokenType::Colon);

    m_sprite_id = is.match_integer();

    // TODO these need to be passed back to the main structure somehow.
    std::vector<std::shared_ptr<Record>> sprite_list;

    is.match(TokenType::OpenBrace);
    while (is.peek().type != TokenType::CloseBrace)
    {
        std::shared_ptr<Record> record;

        // SpriteIndexRecords generally "contain" one or several real sprite images (zoom levels),
        // But can also contain a single binary sound effect, which is itself found in the graphics
        // section of the GRF.
        if (is.peek().type == TokenType::OpenBracket)
        {
            // Need to create RealSpriteRecord     
            // Size and compression are place holder values to be read from the token stream.
            record = std::make_shared<RealSpriteRecord>(m_sprite_id, 0, 0);
        }
        else
        {
            // ActionFF is a sound effect.
            record = std::make_shared<ActionFFRecord>();
        }
        
        sprite_list.push_back(record);
        record->parse(is);
    }

    is.match(TokenType::CloseBrace);
}
