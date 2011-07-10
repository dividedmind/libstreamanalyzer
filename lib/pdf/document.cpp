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
    parser->seek(parser->size());
    parser->findBackwards("startxref");
}

shared_ptr<Object> Document::dereference(Reference* ref)
{
/*    int index = ref->index();
    
    if (!objects[index]) {
        parser->seek(xRefTable->offset(index));
        shared_ptr<IndirectObject> indirect(parser->parseIndirectObject());
        if (index != indirect->index())
            throw Parser::ParseError("indirect object index mismatch");
        objects[index] = indirect;
    }
    
    return objects[index];*/
}

