/*
    Strigi desktop search
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

#include "pdftext.h"

void PdfText::push(std::string text)
{
    texts.push_back(text);
}

PdfText& PdfText::operator+= ( const PdfText& other )
{
    for (TextList::const_iterator it = other.texts.begin(); it != other.texts.end(); ++it)
        texts.push_back(*it);
    
    return *this;
}

void PdfText::dump(PdfText::TextHandler *handler) const
{
    std::string result;
    for (TextList::const_iterator it = texts.begin(); it != texts.end(); ++it)
        handler->handle(*it);
}
