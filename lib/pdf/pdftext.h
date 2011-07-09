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


#ifndef PDFTEXT_H
#define PDFTEXT_H
#include <list>
#include <string>

#include <strigi/streambase.h>

class PdfText
{
public:
    class TextHandler;
    
    void push(const std::string &text);
    void setFont(const std::string &font);

    PdfText &operator +=(const PdfText &other);
    void dump(TextHandler *handler) const;
    
    class TextHandler {
    public:
        virtual ~TextHandler() {}
        virtual Strigi::StreamStatus handle(const std::string& s) = 0;
    };
    
private:
    std::string currentFont;

    struct Element {
        Element(const std::string &_text, const std::string &_font) : text(_text), font(_font) {}
        const std::string text;
        const std::string font;
    };
    typedef std::list<Element> TextList;
    TextList texts;
};

#endif // PDFTEXT_H
