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

#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_expect.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_domain.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include "xreftable.h"

namespace Pdf { namespace Parser {
    namespace qi = boost::spirit::qi;
    using qi::uint_;
    using qi::char_;
    using namespace qi::labels;
    using qi::rule;
    
    const skip_rule xrefentry =
        uint_ > uint_ > char_("nf");
    const rule<StreamWrapper::Iterator, qi::locals<unsigned int>, simple_rule> xreftable_ =
        "xref" > uint_ > uint_[_a = _1] > repeat(_a)[xrefentry];
    const skip_rule xreftable = xreftable_;
}}
