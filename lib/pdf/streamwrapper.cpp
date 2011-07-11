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

#include "streamwrapper.h"

Pdf::StreamWrapper::StreamWrapper(Strigi::StreamBase<char> *stream) : stream(stream)
{
}

void Pdf::StreamWrapper::seek(int64_t position)
{
    if (position == stream->position())
        return;
    
    if (position < stream->position())
        stream->reset(position);
    else
        stream->skip(position - stream->position());
}

int64_t Pdf::StreamWrapper::size() const
{
    return stream->size();
}

char Pdf::StreamWrapper::getChar()
{
    const char *buf;
    stream->read(buf, 1, 1);
    return *buf;
}

void Pdf::StreamWrapper::putChar()
{
    stream->reset(stream->position() - 1);
}

Pdf::StreamWrapper::Iterator::Iterator(Pdf::StreamWrapper* parent, int position) : parent(parent), position(position) {}

Pdf::StreamWrapper::Iterator Pdf::StreamWrapper::Iterator::operator++(int)
{
    Iterator result(*this);
    position++;
    return result;
}

Pdf::StreamWrapper::Iterator &Pdf::StreamWrapper::Iterator::operator++()
{
    position++;
    return *this;
}

char Pdf::StreamWrapper::Iterator::operator*() const
{
    parent->seek(position);

    char c = parent->getChar();
    parent->putChar();
    return c;
}

bool Pdf::StreamWrapper::Iterator::operator==(const Pdf::StreamWrapper::Iterator& other) const
{
    return other.parent == parent && other.position == position;
}

bool Pdf::StreamWrapper::Iterator::operator!=(const Pdf::StreamWrapper::Iterator& other) const
{
    return !(*this == other);
}

Pdf::StreamWrapper::Iterator Pdf::StreamWrapper::end()
{
    return Iterator(this, size());
}

Pdf::StreamWrapper::Iterator Pdf::StreamWrapper::here()
{
    return Iterator(this, stream->position());
}
