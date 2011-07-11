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

#include "parser.h"

Pdf::Parser::Parser(Strigi::StreamBase<char> *stream) : stream(stream)
{
}

void Pdf::Parser::seek(int64_t position)
{
    if (position == stream->position())
        return;
    
    if (position < stream->position())
        stream->reset(position);
    else
        stream->skip(position - stream->position());
}

int64_t Pdf::Parser::size() const
{
    return stream->size();
}

char Pdf::Parser::getChar()
{
    const char *buf;
    stream->read(buf, 1, 0);
    return *buf;
}

void Pdf::Parser::putChar()
{
    stream->reset(stream->position() - 1);
}

Pdf::Parser::ConstIterator::ConstIterator(Parser* parent, int position) : parent(parent), position(position) {}

char Pdf::Parser::ConstIterator::operator++(int)
{
    parent->seek(position++);
    return parent->getChar();
}

char Pdf::Parser::ConstIterator::operator++()
{
    parent->seek(++position);
    char c = parent->getChar();
    parent->putChar();
    return c;
}

char Pdf::Parser::ConstIterator::operator*() const
{
    parent->seek(position);

    char c = parent->getChar();
    parent->putChar();
    return c;
}

bool Pdf::Parser::ConstIterator::operator==(const Pdf::Parser::ConstIterator& other) const
{
    return other.parent == parent && other.position == position;
}

bool Pdf::Parser::ConstIterator::operator!=(const Pdf::Parser::ConstIterator& other) const
{
    return !(*this == other);
}

Pdf::Parser::ConstIterator Pdf::Parser::end()
{
    return ConstIterator(this, size());
}

Pdf::Parser::ConstIterator Pdf::Parser::here()
{
    return ConstIterator(this, stream->position());
}
