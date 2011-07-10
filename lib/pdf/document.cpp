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

#include "dictionary.h"
#include "indirectobject.h"
#include "number.h"
#include "parser.h"
#include "reference.h"
#include "xreftable.h"

#include "document.h"

using namespace Pdf;

shared_ptr<Document> Document::from(Strigi::StreamBase< char >* stream)
{
    shared_ptr<Parser> parser(new Parser(stream));
    shared_ptr<Document> document(new Document(parser));
    return document;
}

Document::Document(shared_ptr<Parser> parser) : parser(parser)
{
    parser->resetStream(parser->fileSize());
    parser->findBackwards("startxref");
    parser->checkKeyword("startxref");
    startXRef = parser->parseSimpleNumber();
    
    parser->findBackwards("trailer");
    parser->checkKeyword("trailer");
    parser->skipWhitespaceAndComments();
    if (parser->getChar() != '<' || parser->getChar() != '<')
        throw Parser::ParseError("expected trailer dictionary");
    
    
    trailer = boost::shared_ptr<Pdf::Dictionary>(parser->parseDictionary());
    if (trailer->count("Prev"))
        std::cerr << "PdfParser warning: multiple xref tables not yet supported" << std::endl;
    
    int size = dynamic_cast<const Pdf::Number &>(trailer->get("Size"));
    objects.resize(size);
    xRefTable = boost::shared_ptr<Pdf::XRefTable>(new Pdf::XRefTable(size));
    parser->resetStream(startXRef);
    xRefTable->parse(parser.get());

    boost::shared_ptr<Pdf::Object> root = trailer->at("Root");
    if (Pdf::Reference *ref = dynamic_cast<Pdf::Reference *>(root.get()))
        root = dereference(ref);

    std::cerr << *root;
}

shared_ptr<Object> Document::dereference(Reference* ref)
{
    int index = ref->index();
    
    if (!objects[index]) {
        parser->resetStream(xRefTable->offset(index));
        shared_ptr<IndirectObject> indirect(parser->parseIndirectObject());
        if (index != indirect->index())
            throw Parser::ParseError("indirect object index mismatch");
        objects[index] = indirect;
    }
    
    return objects[index];
}

