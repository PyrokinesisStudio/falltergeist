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

// Related headers
#include "../UI/TextArea.h"

// C++ standard includes
#include <algorithm>
#include <sstream>

// Falltergeist includes
#include "../CrossPlatform.h"
#include "../Event/Mouse.h"
#include "../Font.h"
#include "../FontString.h"
#include "../Game/Game.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Texture.h"
#include "../ResourceManager.h"
#include "../UI/TextSymbol.h"
#include "../Logger.h"

// Third party includes
#include <SDL.h>

namespace Falltergeist
{
namespace UI
{

TextArea::TextArea(const Point& pos) : Base(pos)
{
    _timestampCreated = SDL_GetTicks();
}

TextArea::TextArea(int x, int y) : TextArea(Point(x, y))
{
}

TextArea::TextArea(const std::string& text, const Point& pos) : Base(pos)
{
    _timestampCreated = SDL_GetTicks();
    setText(text);
}

TextArea::TextArea(const std::string& text, int x, int y) : TextArea(text, Point(x, y))
{
}

TextArea::TextArea(const TextArea& textArea, Point pos) : Base(pos)
{
    _timestampCreated = textArea._timestampCreated;
    _text = textArea._text;
    _font = textArea._font;
    _backgroundColor = textArea._backgroundColor;
    _size = textArea._size;
    _horizontalAlign = textArea._horizontalAlign;
    _verticalAlign = textArea._verticalAlign;
    _wordWrap = textArea._wordWrap;
}

TextArea::~TextArea()
{
}

void TextArea::appendText(const std::string& text)
{
    _text += text;
    _changed = true;
    _lines.clear();
}

TextArea::HorizontalAlign TextArea::horizontalAlign() const
{
    return _horizontalAlign;
}

void TextArea::setHorizontalAlign(HorizontalAlign align)
{
    if (_horizontalAlign == align) return;
    _horizontalAlign = align;
    _changed = true;
}

TextArea::VerticalAlign TextArea::verticalAlign() const
{
    return _verticalAlign;
}

void TextArea::setVerticalAlign(VerticalAlign align)
{
    if (_verticalAlign == align) return;
    _verticalAlign = align;
    _changed = true;
}

void TextArea::setText(const std::string& text)
{
    _text = text;
    _changed = true;
    _lines.clear();
}

Font* TextArea::font()
{
    if (!_font)
    {
        _font = ResourceManager::getInstance()->font();
    }
    return _font;
}

void TextArea::setFont(Font* font)
{
    _font = font;
    _changed = true;
    _lines.clear();
}

void TextArea::setFont(const std::string& fontName, unsigned int color)
{
    setFont(ResourceManager::getInstance()->font(fontName, color));
}

std::string TextArea::fontName()
{
    return font()->filename();
}

void TextArea::setWordWrap(bool wordWrap)
{
    if (_wordWrap == wordWrap) return;
    _wordWrap = wordWrap;
    _changed = true;
    _lines.clear();
}

bool TextArea::wordWrap() const
{
    return _wordWrap;
}

void TextArea::setOutline(bool outline)
{
    _outlineColor = outline ? 0x000000ff : 0;
}

bool TextArea::outline() const
{
    return _outlineColor != 0;
}

void TextArea::setOutlineColor(unsigned int color)
{
    _outlineColor = color;
    _changed = true;
}

unsigned int TextArea::outlineColor() const
{
    return _outlineColor;
}

int TextArea::lineOffset() const
{
    return _lineOffset;
}

void TextArea::setLineOffset(int offset)
{
    _lineOffset = offset;
    _changed = true;
}

Size TextArea::size() const
{
    return Size(
        _size.width() ?: _calculatedSize.width(),
        _size.height() ?: _calculatedSize.height()
    );
}

Size TextArea::textSize()
{
    _updateSymbols();
    return _calculatedSize;
}

bool TextArea::overflown()
{
    return _overflown;
}

int TextArea::numLines()
{
    return _lines.size();
}

void TextArea::setSize(const Size& size)
{
    if (_size == size) return;
    _size = size;
    _changed = true;
}

void TextArea::setWidth(int width)
{
    setSize({width, _size.height()});
}

// TODO: refactoring of this function would be nice
void TextArea::_updateSymbols()
{
    if (!_changed) return;

    _symbols.clear();

    if (_text.empty())
    {
        _changed = false;
        return;
    }

    unsigned maxLines = _size.height()
           ? (unsigned)(_size.height() + font()->verticalGap()) / (font()->height() + font()->verticalGap())
           : 0u;

    if (_lines.empty())
    {
        _lines = _generateLines(0, 0);
    }

    // TODO: maybe move this code to another class?

    // at positive offset, skip number of first lines
    int lineOffset = _lineOffset > 0 ? _lineOffset : 0;
    auto lineBegin = _lines.cbegin() + lineOffset;
    auto lineEnd = std::min(_lines.cbegin() + lineOffset + maxLines, _lines.cend());

    int numLines = std::distance(lineBegin, lineEnd);

    // Calculating textarea sizes if needed
    _calculatedSize.setWidth(std::max_element(lineBegin, lineEnd)->width);
    _calculatedSize.setHeight(numLines*font()->height() + (numLines - 1)*font()->verticalGap());

    // Alignment and outlining
    auto outlineFont = (_outlineColor != 0)
                       ? ResourceManager::getInstance()->font(font()->filename(), _outlineColor)
                       : nullptr;

    Point offset;
    // on negative offset, add padding at the top
    if (_lineOffset < 0)
    {
        offset.setY((font()->height() + font()->verticalGap()) * (- _lineOffset));
    }
    // TODO: Y coords are already off-set in _generateLines!
    for (auto it = lineBegin; it != lineEnd; ++it)
    {
        auto& line = *it;
        if (_horizontalAlign != HorizontalAlign::LEFT)
        {
            offset.setX((_size.width() ? _size.width() : _calculatedSize.width()) - line.width);
            if (_horizontalAlign == HorizontalAlign::CENTER)
            {
                offset.rx() /= 2;
            }
        }

        for (TextSymbol symbol : line.symbols)
        {
            symbol.setPosition(symbol.position() + offset);
            // outline symbols
            if (_outlineColor != 0)
            {
                _addOutlineSymbol(symbol, outlineFont,  0, -1);
                _addOutlineSymbol(symbol, outlineFont,  0,  1);
                _addOutlineSymbol(symbol, outlineFont, -1,  0);
                _addOutlineSymbol(symbol, outlineFont, -1, -1);
                _addOutlineSymbol(symbol, outlineFont, -1,  1);
                _addOutlineSymbol(symbol, outlineFont,  1,  0);
                _addOutlineSymbol(symbol, outlineFont,  1, -1);
                _addOutlineSymbol(symbol, outlineFont,  1,  1);
            }
        
            _symbols.push_back(symbol);
        }
    }
    _changed = false;
}

std::vector<TextArea::Line> TextArea::_generateLines(unsigned int maxLines, int lineOffset)
{
    //static_assert(std::is_trivially_copyable<TextSymbol>(), "TextSymbol should be trivially copyable.");

    std::vector<Line> lines;
    lines.resize(lineOffset < 0 ? 1 + (size_t)(- lineOffset) : 1);
    
    int x = 0, y = 0, wordWidth = 0;
    int lineBreaks = 0;
    
    // Parsing lines of text
    // Cutting lines when it is needed (\n or when exceeding _width)
    std::istringstream istream(_text);
    std::string word;
    auto aFont = font();
    auto glyphs = aFont->aaf()->glyphs();
    
    while (istream >> word)
    {
        // calculate word width
        wordWidth = 0;
        for (unsigned char ch : word)
        {
            wordWidth += glyphs->at(ch)->width() + aFont->horizontalGap();
        }
        // switch to next line if word is too long
        if (_wordWrap && _size.width() && (x + wordWidth) > _size.width())
        {
            word = '\n' + word;
        }
        // include whitespaces
        while (!istream.eof() && isspace((int)istream.peek()))
        {
            word.push_back((char)istream.get());
        }
        // place the word
        for (unsigned char ch : word)
        {
            if (ch == ' ')
            {
                x += aFont->aaf()->spaceWidth() + aFont->horizontalGap();
            }

            if (ch == '\n' || (_wordWrap && _size.width() && x >= _size.width()))
            {
                if (maxLines && lines.size() >= maxLines)
                {
                    _overflown = true;
                    return lines;
                }

                if (lineBreaks >= lineOffset)
                {
                    lines.back().width = x;
                    x = 0;
                    y += aFont->height() + aFont->verticalGap();
                    lines.emplace_back();
                }
                ++lineBreaks;
            }

            if (ch == ' ' || ch == '\n')
                continue;

            Line& line = lines.back();
            if (lineBreaks >= lineOffset)
            {
                TextSymbol symbol(ch, {x, y});
                symbol.setFont(aFont);
                line.symbols.push_back(symbol);
                x += glyphs->at(ch)->width() + aFont->horizontalGap();
                line.width = x;
            }
        }
    }
    return lines;
}

void TextArea::_addOutlineSymbol(const TextSymbol& symb, Font* font, int32_t ofsX, int32_t ofsY)
{
    _symbols.emplace_back(symb.chr(), symb.position() + Point(ofsX, ofsY));
    _symbols.back().setFont(font);
}

std::string TextArea::text() const
{
    return _text;
}

unsigned int TextArea::timestampCreated() const
{
    return _timestampCreated;
}

void TextArea::render(bool eggTransparency)
{
    if (_changed)
        _updateSymbols();

    auto pos = position();
    for (auto& symbol : _symbols)
    {
        symbol.render(pos);
    }
}

TextArea& TextArea::operator<<(const std::string& text)
{
    appendText(text);
    return *this;
}

TextArea& TextArea::operator<<(unsigned value)
{
    appendText(std::to_string(value));
    return *this;
}

TextArea& TextArea::operator<<(signed value)
{
    appendText(std::to_string(value));
    return *this;
}

TextArea& TextArea::operator=(const std::string& text)
{
    setText(text);
    return *this;
}

TextArea& TextArea::operator=(unsigned value)
{
    setText(std::to_string(value));
    return *this;
}

TextArea& TextArea::operator=(signed value)
{
    setText(std::to_string(value));
    return *this;
}

TextArea& TextArea::operator+=(const std::string& text)
{
    appendText(text);
    return *this;
}

TextArea& TextArea::operator+=(unsigned value)
{
    appendText(std::to_string(value));
    return *this;
}

TextArea& TextArea::operator+=(signed value)
{
    appendText(std::to_string(value));
    return *this;
}

unsigned int TextArea::pixel(const Point& pos)
{
    if (!Rect::inRect(pos, this->size()))
    {
        return 0; // transparent
    }
    return 0xFFFFFFFF; // white color
}

void TextArea::handle(Event::Event* event)
{
    Base::handle(event);
    if (auto mouseEvent = dynamic_cast<Event::Mouse*>(event))
    {
        mouseEvent->setHandled(false);
        mouseEvent->setObstacle(false);
    }
}

}
}
