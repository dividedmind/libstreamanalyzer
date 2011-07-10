/*
    Strigi PDF parser.
    Copyright (C) 2011  Rafał Rzepecki <divided.mind@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <cmath>
#include <sstream>

#include <strigi/kmpsearcher.h>
#include <strigi/subinputstream.h>

#include "array.h"
#include "dictionary.h"
#include "indirectobject.h"
#include "name.h"
#include "number.h"
#include "reference.h"
#include "stream.h"
#include "string.h"
#include "xreftable.h"

#include "parser.h"

using namespace Pdf;

#define forever for(;;)

Parser::Parser(Strigi::StreamBase<char> *stream) : stream(stream)
{
}

void Parser::seek(int64_t position)
{
    if (position == stream->position())
        return;
    
    if (position < stream->position())
        stream->reset(position);
    else
        stream->skip(position - stream->position());
}

int64_t Parser::size() const
{
    return stream->size();
}

void Parser::findBackwards(const char *needle)
{
    int wrap = 4;
    int64_t startPosition = stream->position();
    
    Strigi::KmpSearcher searcher(needle);

    forever {
        seek(startPosition - wrap);
        const char *buffer;
        stream->read(buffer, wrap, wrap);

        const char *final = 0;
        const char *result = buffer - 1;
        
        while ((result = searcher.search(result + 1, wrap - (result - buffer + 1))))
            final = result;
        if (final) {
            seek(startPosition - wrap + final - buffer);
            return;
        } else {
            wrap *= 2;
            if (wrap > startPosition) {
                wrap = startPosition;
            } else if (wrap == startPosition)
                throw ParseError("couldn't find keyword");
        }
    }
}

char Parser::getChar()
{
    const char *buf;
    stream->read(buf, 1, 0);
    return *buf;
}

void Parser::putChar()
{
    stream->reset(stream->position() - 1);
}

Parser::ConstIterator::ConstIterator(Parser* parent, int position) : parent(parent), position(position) {}

char Parser::ConstIterator::operator++(int)
{
    parent->seek(position++);
    return parent->getChar();
}

char Parser::ConstIterator::operator++()
{
    parent->seek(++position);
    char c = parent->getChar();
    parent->putChar();
    return c;
}

char Parser::ConstIterator::operator*() const
{
    parent->seek(position);

    char c = parent->getChar();
    parent->putChar();
    return c;
}

bool Parser::ConstIterator::operator==(const Pdf::Parser::ConstIterator& other) const
{
    return other.parent == parent && other.position == position;
}

bool Parser::ConstIterator::operator!=(const Pdf::Parser::ConstIterator& other) const
{
    return !(*this == other);
}

Parser::ConstIterator Parser::end()
{
    return ConstIterator(this, size());
}

Parser::ConstIterator Parser::here()
{
    return ConstIterator(this, stream->position());
}
