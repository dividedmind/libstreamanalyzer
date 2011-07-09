/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "pdfparser.h"

#define forever for(;;)

PdfParser::PdfParser() :streamhandler(0), texthandler(0), buffer(0) {
}

Strigi::StreamStatus
PdfParser::parse(Strigi::StreamBase<char>* stream) throw (ParseError) {
    this->stream = stream;
    stream->reset(0);

    skipWhitespaceAndComments();
    
    this->stream = 0;
    return stream->status();
}

/**
 * Skip any whitespace and comments in the stream.
 * It will be positioned at the next non-whitespace character.
 */
void PdfParser::skipWhitespaceAndComments()
{
    char c;
    forever {
        c = getChar();
        if (c == '%')
            skipCommentBody();
        else if (isSpace(c));
        else break;
    }
    
    putChar();
}

/**
 * Get the next character from the stream.
 */
char PdfParser::getChar()
{
    if (!buffer || pos == bufEnd)
        fillBuffer(1);
    return *(pos++);
}

/**
 * Fill the buffer to at least @arg minChars bytes.
 */
void PdfParser::fillBuffer(int minChars)
{
    bufferStart = stream->position();
    if (stream->read(buffer, minChars, 0) < minChars)
        throw stream->status();
}

/**
 * Check if character is one of PDF whitespace characters, that is 
 * null, horizontal tab, line feed, form feed, carriage return, or space.
 * 
 * (Table 1 of the PDF spec.)
 */
bool PdfParser::isSpace(char c)
{
    switch (c) {
        case 0:
        case '\x09':
        case '\n':
        case '\f':
        case '\r':
        case ' ':
            return true;
        default:
            return false;
    };
}

/** Put back last character into the buffer.
 * Rewind the stream one char if at the buffer start.
 */
void PdfParser::putChar()
{
    if (pos == buffer) {
        stream->reset(--bufferStart);
        fillBuffer(1);
    } else
        pos--;
}

void PdfParser::skipCommentBody()
{
    forever {
        switch (getChar()) {
            case '\r':
            case '\n':
                break;
            default:;
        }
    }
}
