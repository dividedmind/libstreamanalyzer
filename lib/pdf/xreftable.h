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

#ifndef PDF_XREFTABLE_H
#define PDF_XREFTABLE_H

#include <vector>
#include <iostream>

namespace Pdf {
//

class Parser;

class XRefTable : public std::vector< std::pair<int, int> > {
public:
    XRefTable(int size) : std::vector< std::pair<int, int> >(size) {}
    void parse(Pdf::Parser* parser);
    int offset(int index) const;
};

std::ostream &operator <<(std::ostream &stream, const XRefTable &table);

}

#endif // PDF_XREFTABLE_H