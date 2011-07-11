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
#include <sstream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "parser.h"
#include "grammar.h"

namespace qi = boost::spirit::qi;
namespace qis = boost::spirit::standard;
namespace phx = boost::phoenix;

namespace Pdf { namespace Grammar {
//
typedef qi::rule<Parser::ConstIterator> simple_rule;
typedef qi::rule<Parser::ConstIterator, simple_rule> skipping_rule;

simple_rule eol = qi::lit('\r') || qi::lit('\n');
simple_rule whitespace = qi::char_("\t\n\f\r ") | qi::lit('\0');
simple_rule delimiter = qi::char_("()<>[]{}/%");
simple_rule regular = !whitespace >> !delimiter >> qi::char_;
simple_rule comment = '%' >> *(qi::char_ - eol) >> eol;
simple_rule skipper = whitespace | comment;

struct document : qi::grammar<Parser::ConstIterator, simple_rule>
{
    document() : base_type(pdf, "pdf")
    {
        using qi::int_;
        using qi::lit;
        using qis::xdigit;
        
        pdf = +indirect_object;
        indirect_object = int_ > int_ > "obj" > object > "endobj";
        object = stream | dictionary | name | reference | number | array | string;
        dictionary = "<<" > *(name > object) > ">>";
        name_escape = '#' > xdigit > xdigit;
        name = qi::lexeme['/' > *(name_escape | regular)];
        number = qi::real_parser< double, qi::strict_real_policies<double> >() | int_;
        stream = dictionary >> qi::lexeme["stream" > eol > *(!(eol >> lit("endstream")) > qi::byte_) > eol > "endstream"];
        reference = int_ >> int_ >> 'R';
        array = '[' > *object > ']';
        string_escape = '\\' >> -(qi::char_("nrtbf()\\") | qi::repeat(1, 3)[qi::char_('0', '7')] | eol);
        string = '(' > *(string_escape | ~qi::lit(')')) > ')';
        
        pdf.name("pdf");
        indirect_object.name("indirect object");
        object.name("object");
        dictionary.name("dictionary");
        name.name("name");
        number.name("number");
        stream.name("stream");
        
        using phx::ref;
        using phx::val;
        using phx::bind;
        using phx::throw_;
        using phx::construct;
        using qi::_4;

        qi::on_error<qi::fail>(pdf, (
            error_stream << val("Expected "),
            error_stream << _4,
            throw_(construct<Parser::ParseError>(bind(&std::stringstream::str, error_stream)))
        ));
        
/*        qi::on_error<qi::fail>(pdf, 
                               let(_a = _4) []);*/
    }
    
    std::stringstream error_stream;
    simple_rule name_escape, string, string_escape;
    skipping_rule pdf, indirect_object, object, dictionary, name, number, stream, reference, array;
};

bool parse(boost::shared_ptr<Parser> stream)
{
    
    using qi::lit;
    
    document pdf;
    Parser::ConstIterator it = stream->here();
    return qi::phrase_parse(it, stream->end(), pdf, skipper);
}

}}
