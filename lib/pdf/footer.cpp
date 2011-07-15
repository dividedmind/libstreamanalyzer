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

#include <iostream>

#include <boost/spirit/include/qi_parse_attr.hpp>

#include "xreftable.h"

#include "footer.h"

namespace Pdf {
    namespace Parser {
        const footer_type footer = xreftable;
    }
    
    namespace Footer {
        bool parse(StreamWrapper::Iterator begin, StreamWrapper::Iterator end)
        {
            XRefTable xreftable;
            if (boost::spirit::qi::phrase_parse(begin, end, Parser::xreftable, Parser::skipper, xreftable))
                std::cerr << "parse successful" << std::endl << xreftable.size() << xreftable << std::endl;
            else
                return false;
            return true;
        }
    }
}
