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

#include "parser.h"

#include "document.h"
#include "grammar.h"

using namespace Pdf;

using boost::shared_ptr;

shared_ptr<Document> Document::from(Strigi::StreamBase< char >* stream)
{
    shared_ptr<Parser> parser(new Parser(stream));
    shared_ptr<Document> document(new Document(parser));
    return document;
}

Document::Document(shared_ptr<Parser> parser) : parser(parser)
{
    parser->seek(0);
    if (!Grammar::parse(parser))
        throw Parser::ParseError("parse error");
}
