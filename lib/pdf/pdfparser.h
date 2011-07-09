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

#ifndef PDFPARSER_H
#define PDFPARSER_H

#include <strigi/streambase.h>

namespace Pdf {
    class Object;
    class Dictionary;
    class Name;
};

class PdfParser {
public:
    class StreamHandler {
    public:
        virtual ~StreamHandler() {}
        virtual Strigi::StreamStatus handle(Strigi::StreamBase<char>* s) = 0;
    };
    class TextHandler {
    public:
        virtual ~TextHandler() {}
        virtual Strigi::StreamStatus handle(const std::string& s) = 0;
    };
    
    class ParseError : public std::exception {
    public:
        ParseError(const std::string &msg) : message(msg) {}
        virtual ~ParseError() throw() {}
        
        const char *what() const throw() {
            return message.c_str();
        }
        
    private:
        std::string message;
    };
    
private:
    StreamHandler *streamhandler;
    TextHandler *texthandler;

    Strigi::StreamBase<char> *stream;
    const char *buffer, *pos, *bufEnd;
    int64_t bufferStart;

    void skipWhitespaceAndComments();
    char getChar();
    void skipCommentBody();
    void putChar();
    void fillBuffer(int minChars);
    static bool isSpace(char c);
    void parseIndirectObject();
    int parseNumber();
    void checkKeyword(const char* keyword);
    Pdf::Object* parseObject();
    Pdf::Dictionary* parseDictionary();
    Pdf::Name* parseName();
    char getHexChar();
    static char fromHex(char c);
    static bool isRegular(char c);
    static bool isDelimiter(char c);

public:
    PdfParser();
    /**
     * Parse a PDF file.
     * @arg file Stream representing the file to be parsed.
     * @return Stream status of the file. On successful parsing it should be @ref Strigi::Eof.
     */
    Strigi::StreamStatus parse(Strigi::StreamBase< char >* stream) throw (ParseError);
    void setStreamHandler(StreamHandler* handler) { streamhandler = handler; }
    void setTextHandler(TextHandler* handler) { texthandler = handler; }
};

#endif
