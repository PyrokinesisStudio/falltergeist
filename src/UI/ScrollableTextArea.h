/*
 * Copyright 2012-2015 Falltergeist Developers.
 *
 * This file is part of Falltergeist.
 *
 * Falltergeist is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Falltergeist is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Falltergeist.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FALLTERGEIST_UI_SCROLLABLETEXTAREA_H
#define FALLTERGEIST_UI_SCROLLABLETEXTAREA_H

// C++ standard includes
#include <cstdint>
#include <string>
#include <vector>

// Falltergeist includes
#include "../UI/TextArea.h"

// Third party includes

namespace Falltergeist
{

namespace UI
{

/**
 * Represents a TextArea, where contained text could be scrolled up and down line-wise.
 */
class ScrollableTextArea : public TextArea
{
public:
    using TextArea::TextArea;

    unsigned lineOffset() const;
    void setLineOffset(unsigned);

protected:
    int _lineOffset = 0;
    
    using Line = TextArea::Line;



};

}
}

#endif // FALLTERGEIST_UI_SCROLLABLETEXTAREA_H
