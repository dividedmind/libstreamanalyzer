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

#include "streamwrapper.h"

#include "parser.h"

namespace qi = boost::spirit::qi;
namespace qis = boost::spirit::standard;
namespace phx = boost::phoenix;

namespace Pdf { namespace Parser {
//
typedef qi::rule<StreamWrapper::Iterator> simple_rule;
typedef qi::rule<StreamWrapper::Iterator, simple_rule> skipping_rule;

simple_rule eol = qi::lit('\r') || qi::lit('\n');
simple_rule whitespace = qi::char_("\t\n\f\r ") | qi::lit('\0');
simple_rule delimiter = qi::char_("()<>[]{}/%");
simple_rule regular = !whitespace >> !delimiter >> qi::char_;
simple_rule comment = '%' >> *(!eol >> qi::char_) >> eol;
simple_rule skipper = whitespace | comment;

struct document : qi::grammar<StreamWrapper::Iterator, simple_rule>
{
    document() : base_type(pdf, "pdf")
    {
        using qi::int_;
        using qi::lit;
        using qis::xdigit;
        using qis::digit;
        using qi::repeat;
        using qi::char_;
        using qi::eoi;
        using qi::lexeme;
        using qi::byte_;
        
        pdf = 
            *(*indirect_object >> xreftable >> trailer >> xrefpos) >> eoi;
        indirect_object = 
            int_ > int_ > "obj" > object > "endobj";
        object = 
            stream | dictionary | name | reference | number | array | string;
        dictionary = 
            "<<" > *(name > object) > ">>";
        name_escape = 
            '#' > xdigit > xdigit;
        name = 
            lexeme['/' > *(name_escape | regular)];
        number = 
            qi::real_parser< double, qi::strict_real_policies<double> >() | int_;
        stream = 
            dictionary >> lexeme["stream" > eol > *(!(eol >> lit("endstream")) > byte_) > eol > "endstream"];
        reference = 
            int_ >> int_ >> 'R';
        array = 
            '[' > *object > ']';
        string_escape = 
            '\\' >> -(char_("nrtbf()\\") | repeat(1, 3)[char_('0', '7')] | eol);
        literal_string = 
            '(' > *(literal_string | string_escape | ~lit(')')) > ')';
        string = 
            literal_string | hex_string;
        hex_string = 
            '<' > *xdigit > '>';
        xreftable = 
            lit("xref") > int_ > int_ > eol > *xrefentry;
        xrefentry = 
            repeat(10)[digit] > ' ' > repeat(5)[digit] > ' ' > char_("nf") > (lit(" \r") | lit(" \n") | lit("\r\n"));
        trailer = 
            "trailer" > dictionary;
        xrefpos =
            "startxref" > int_;
        
        pdf.name("pdf");
        indirect_object.name("indirect object");
        object.name("object");
        dictionary.name("dictionary");
        name.name("name");
        number.name("number");
        stream.name("stream");
        xreftable.name("xreftable");
        
        using phx::ref;
        using phx::val;
        using phx::bind;
        using phx::throw_;
        using phx::construct;
        using qi::_4;

        qi::on_error<qi::fail>(pdf, (
            error_stream << val("Expected "),
            error_stream << _4,
            throw_(construct<ParseError>(bind(&std::stringstream::str, error_stream)))
        ));
    }
    
    std::stringstream error_stream;
    simple_rule name_escape, literal_string, string_escape, xrefentry;
    skipping_rule pdf, indirect_object, object, dictionary, name, number, stream, reference, array, string, hex_string, xreftable, trailer, xrefpos;
};

bool parse(StreamWrapper::Iterator begin, StreamWrapper::Iterator end)
{
    document pdf;
    return qi::phrase_parse(begin, end, pdf, skipper);
}

}}
