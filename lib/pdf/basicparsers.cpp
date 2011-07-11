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

#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/qi_sequential_or.hpp>
#include <boost/spirit/include/qi_alternative.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_kleene.hpp>
#include <boost/spirit/include/qi_not_predicate.hpp>

#include "basicparsers.h"

namespace qi = boost::spirit::qi;

namespace Pdf { namespace Parser {
//
const simple_rule eol = qi::lit('\r') || qi::lit('\n');
const simple_rule whitespace = qi::char_("\t\n\f\r ") | qi::lit('\0');
const simple_rule delimiter = qi::char_("()<>[]{}/%");
const simple_rule regular = !whitespace >> !delimiter >> qi::char_;
const simple_rule comment = '%' >> *(!eol >> qi::char_) >> eol;
const simple_rule skipper = whitespace | comment;
}}
