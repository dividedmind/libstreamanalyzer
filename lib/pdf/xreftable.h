/*
 *    Strigi PDF parser.
 *    Copyright (C) 2011  Rafał Rzepecki <divided.mind@gmail.com>
 * 
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 * 
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 * 
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PDF_XREFTABLE_H
#define PDF_XREFTABLE_H

#include <map>
#include <utility>
#include <ostream>

#include "basicparsers.h"

namespace Pdf {
    struct XRefTableIndex {
        unsigned number, generation;
        bool operator<(const XRefTableIndex &other) const;
    };
    
    struct XRefTableEntry {
        unsigned offset;
        bool in_use;
    };
    
    typedef std::map<XRefTableIndex, XRefTableEntry> XRefTable;
    
    std::ostream &operator<<(std::ostream &o, const XRefTable &t);
    std::ostream &operator<<(std::ostream &o, const XRefTableEntry &t);
    std::ostream &operator<<(std::ostream &o, const XRefTableIndex &t);

    namespace Parser {
        typedef boost::spirit::qi::rule<StreamWrapper::Iterator, simple_rule, XRefTable()> xreftable_type;
        extern xreftable_type xreftable;
    }
};

#endif // PDF_XREFTABLE_H
