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

#ifndef PDF_PARSER_H
#define PDF_PARSER_H

#include <boost/shared_ptr.hpp>
#include <strigi/streambase.h>

namespace Pdf {
//

using namespace boost;

class Object;
class Dictionary;
class Name;
class Number;
class Stream;
class String;
class Array;
class Reference;
class XRefTable;
class IndirectObject;

class Parser {
public:
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

    Parser(Strigi::StreamBase<char> *stream);
    
    void skipWhitespaceAndComments();
    char getChar();
    void skipCommentBody();
    void putChar();
    void fillBuffer(int minChars);
    static bool isSpace(char c);
    shared_ptr<IndirectObject> parseIndirectObject();
    int parseSimpleNumber();
    void checkKeyword(const char* keyword);
    Object* parseObject();
    Dictionary* parseDictionary();
    Name* parseName();
    char getHexChar();
    static char fromHex(char c);
    static bool isRegular(char c);
    static bool isDelimiter(char c);
    Number* parseNumber();
    Stream* parseStream(Dictionary* dict);
    String* parseLiteralString();
    int64_t currentPosition();
    int64_t fileSize();
    Object* parseNumberOrReference();
    void resetStream(int64_t position);
    Array* parseArray();
    char getOctalChar();
    String* parseHexString();
    void findBackwards(const char* needle);

private:
    const char *buffer, *pos, *bufEnd;
    int64_t bufferStart;
    Strigi::StreamBase<char> *stream;
};

}

#endif // PDF_PARSER_H
