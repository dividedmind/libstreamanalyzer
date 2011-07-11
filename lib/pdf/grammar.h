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

#ifndef PDF_GRAMMAR_H
#define PDF_GRAMMAR_H

#include <boost/spirit/include/support.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include "parser.h"

namespace Pdf { namespace Grammar {
//
typedef boost::spirit::qi::rule<Pdf::Parser::ConstIterator> RuleParser;

RuleParser makeNewlineParser();
RuleParser makeWhitespaceParser();

BOOST_SPIRIT_DEFINE_TERMINALS(
    (newline)
    (whitespace)
);
}}

namespace boost { namespace spirit {
    template <>
    struct use_terminal<qi::domain, Pdf::Grammar::tag::newline>
    : mpl::true_ {};
    
    template <>
    struct use_terminal<qi::domain, Pdf::Grammar::tag::whitespace>
    : mpl::true_ {};
}}

namespace boost { namespace spirit { namespace qi
{
    template <typename Modifiers>
    struct make_primitive<Pdf::Grammar::tag::newline, Modifiers>
    {
        typedef Pdf::Grammar::RuleParser result_type;
 
        result_type operator()(unused_type, unused_type) const {
            return Pdf::Grammar::makeNewlineParser();
        }
    };

    template <typename Modifiers>
    struct make_primitive<Pdf::Grammar::tag::whitespace, Modifiers>
    {
        typedef Pdf::Grammar::RuleParser result_type;
 
        result_type operator()(unused_type, unused_type) const {
            return Pdf::Grammar::makeWhitespaceParser();
        }
    };
}}}


#endif // PDF_GRAMMAR_H
