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

#ifndef PDF_BASICPARSERS_H
#define PDF_BASICPARSERS_H

#include <boost/proto/proto_fwd.hpp>
#include <boost/spirit/include/qi_rule.hpp>

#include "streamwrapper.h"

namespace Pdf { namespace Parser {
//
typedef boost::spirit::qi::rule<StreamWrapper::Iterator> simple_rule;
typedef boost::spirit::qi::rule<StreamWrapper::Iterator, simple_rule> skip_rule;

extern const simple_rule eol;
extern const simple_rule whitespace;
extern const simple_rule delimiter;
extern const simple_rule regular;
extern const simple_rule comment;
extern const simple_rule skipper;
}}

#endif // PDF_BASICPARSERS_H
