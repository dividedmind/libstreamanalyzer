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

#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>

#include "parser.h"
#include "grammar.h"

namespace qi = boost::spirit::qi;

namespace Pdf {
bool Grammar::parse(boost::shared_ptr<Parser> stream)
{
    using namespace qi;
    
    auto eol = lit('\r') || lit('\n');
    auto whitespace = char_("\t\n\f\r ") | lit('\0');
    rule<Parser::ConstIterator> comment = '%' >> *(char_ - eol) >> eol;
    
    auto it = stream->here();

    bool result = qi::parse(it, stream->end(), comment >> comment);
    
    return result;
}
}
