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

#include <algorithm>
#include <iostream>
#include <iterator>

#include "xreftable.h"
#include "parser.h"

using namespace Pdf;
using namespace std;

void XRefTable::parse(Pdf::Parser* parser)
{
    copy(parser->here(), parser->end(), ostream_iterator<char>(cerr, ""));
/*    Parser::ConstIterator it = parser->current();
    
    parser->checkKeyword("xref");
    int index = parser->parseSimpleNumber();
    int count = parser->parseSimpleNumber();
    
    for (; count; --count) {
        if (index == size())
            break;
        
        int offset = parser->parseSimpleNumber();
        int generation = parser->parseSimpleNumber();
        parser->skipWhitespaceAndComments();
        if (parser->getChar() == 'n')
            at(index) = std::pair<int, int>(generation, offset);
        index++;
    }*/
}

std::ostream& Pdf::operator<<(std::ostream& stream, const Pdf::XRefTable& table)
{
    for (unsigned i = 0; i < table.size(); i++)
        stream << i << '\t' << table[i].first << '\t' << table[i].second << std::endl;
    return stream;
}

int XRefTable::offset(int index) const
{
    return at(index).second;
}