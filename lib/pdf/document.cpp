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

#include <boost/spirit/include/qi_parse_attr.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_difference.hpp>
#include <boost/spirit/include/qi_kleene.hpp>

#include "parser.h"

#include "document.h"
#include "grammar.h"

using namespace Pdf;

namespace qi = boost::spirit::qi;
using boost::shared_ptr;

shared_ptr<Document> Document::from(Strigi::StreamBase< char >* stream)
{
    shared_ptr<Parser> parser(new Parser(stream));
    shared_ptr<Document> document(new Document(parser));
    return document;
}

Document::Document(shared_ptr<Parser> parser) : parser(parser)
{
    readFooter();
}

void Document::readFooter()
{
    const int MIN_FOOTER_SIZE = 17;
    
    uint64_t offset;
    uint64_t size = parser->size();

    for (offset = MIN_FOOTER_SIZE; offset <= size; ++offset) {
        parser->seek(size - offset);
        if (parseFooter())
            break;
    }
    
    if (offset > size)
        throw Parser::ParseError("couldn't find file footer");
    
    std::cerr << startXRef << std::endl;
}

bool Document::parseFooter()
{
    using qi::ulong_;
    using boost::phoenix::ref;
    using qi::parse;
    using qi::eoi;
    using Grammar::newline;
    using Grammar::whitespace;

    Parser::ConstIterator it = parser->here();
    const qi::rule<Parser::ConstIterator, unsigned long()> footer = 
        *(whitespace - newline) >> newline >>
        "startxref" >> newline >> 
        ulong_ >> newline >> 
        "%%EOF" >> -newline >> eoi;
    
    return qi::parse(it, parser->end(), footer, startXRef);
}
