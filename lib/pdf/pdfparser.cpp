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

#include <math.h>
#include <sstream>

#include "pdfparser.h"
#include "dictionary.h"
#include "name.h"
#include "number.h"

#define forever for(;;)

PdfParser::PdfParser() :streamhandler(0), texthandler(0), buffer(0) {
}

Strigi::StreamStatus
PdfParser::parse(Strigi::StreamBase<char>* stream) throw (ParseError) {
    this->stream = stream;
    stream->reset(0);

    try {
        forever parseIndirectObject();
    } catch (Strigi::StreamStatus s) {}
    catch (ParseError err) {
        stream->reset(bufferStart + (pos - buffer));
        std::stringstream ss;
        ss << err.what() << "; file position: " << stream->position();
        throw ParseError(ss.str());
    };
    
    this->stream = 0;
    return stream->status();
}

/**
 * Skip any whitespace and comments in the stream.
 * It will be positioned at the next non-whitespace character.
 */
void PdfParser::skipWhitespaceAndComments()
{
    char c;
    forever {
        c = getChar();
        if (c == '%')
            skipCommentBody();
        else if (isSpace(c));
        else break;
    }
    
    putChar();
}

/**
 * Get the next character from the stream.
 */
char PdfParser::getChar()
{
    if (!buffer || pos == bufEnd)
        fillBuffer(1);
    return *(pos++);
}

/**
 * Fill the buffer to at least @arg minChars bytes.
 */
void PdfParser::fillBuffer(int minChars)
{
    bufferStart = stream->position();
    int readChars;
    if ((readChars = stream->read(buffer, minChars, 0)) < minChars)
        throw stream->status();
    pos = buffer;
    bufEnd = buffer + readChars;
}

/**
 * Check if character is one of PDF whitespace characters, that is 
 * null, horizontal tab, line feed, form feed, carriage return, or space.
 * 
 * (Table 1 of the PDF spec.)
 */
bool PdfParser::isSpace(char c)
{
    switch (c) {
        case 0:
        case '\x09':
        case '\n':
        case '\f':
        case '\r':
        case ' ':
            return true;
        default:
            return false;
    };
}

/** Put back last character into the buffer.
 * Rewind the stream one char if at the buffer start.
 */
void PdfParser::putChar()
{
    if (pos == buffer) {
        stream->reset(--bufferStart);
        fillBuffer(1);
    } else
        pos--;
}

/**
 * Skips comment body (up to first newline character).
 */
void PdfParser::skipCommentBody()
{
    forever switch (getChar()) {
        case '\r':
        case '\n':
            return;
        default:;
    }
}

/**
 * Parse an indirect object and put it in the indirect object table.
 * PDF spec 7.3.10.
 */
void PdfParser::parseIndirectObject()
{
    int objectNumber = parseSimpleNumber();
    int generationNumber = parseSimpleNumber();
    checkKeyword("obj");
    
    Pdf::Object *object = parseObject();
}

/**
 * Parse a simple, nonnegative integer number.
 */
int PdfParser::parseSimpleNumber()
{
    skipWhitespaceAndComments();
    int result = 0;
    bool atLeastOne = false;
    forever switch (char c = getChar()) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            result *= 10;
            result += c - '0';
            atLeastOne = true;
            continue;
        default:
            putChar();
            if (!atLeastOne)
                throw ParseError("expected a number");
            return result;
    }
}

/**
 * Ensures that @arg keyword comes next in stream.
 * Throws a ParseError if it doesn't.
 */
void PdfParser::checkKeyword(const char* keyword)
{
    skipWhitespaceAndComments();
    const char *i;
    for (i = keyword; *i && getChar() == *i; i++);
    if (*i) {
        std::stringstream ss;
        ss << "expected keyword: " << keyword;
        throw ParseError(ss.str());
    }
}

/**
 * Parse a PDF object occuring in the stream.
 * Returns a newly created object. User should take its ownership.
 */
Pdf::Object *PdfParser::parseObject()
{
    skipWhitespaceAndComments();
    switch (getChar()) {
        case '<':
            if (getChar() == '<')
                return parseDictionary();
        case '/':
            return parseName();
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case '.':
            putChar();
            return parseNumber();
        default:
            throw ParseError("unknown object type");
    }
}

/**
 * Parse a dictionary object.
 * PDF spec 7.3.7.
 */
Pdf::Dictionary *PdfParser::parseDictionary()
{
    Pdf::Dictionary *dict = new Pdf::Dictionary;
    forever {
        Pdf::Name *name = dynamic_cast<Pdf::Name *>(parseObject());
        if (!name) {
            delete dict;
            throw ParseError("expected name");
        }
        
        Pdf::Object *value = parseObject();
        (*dict)[*name] = value;
        delete name;
    }        
}

/**
 * Parse a name object.
 * PDF spec 7.3.5.
 */
Pdf::Name *PdfParser::parseName()
{
    std::string s;
    forever {
        char c = getChar();
        if (c == '#')
            c = getHexChar();
        else if(!isRegular(c)) {
            putChar();
            break;
        }
        s += c;
    }
    
    return new Pdf::Name(s);
}

/**
 * Read two hexadecimal digits and return character corresponding to that number.
 */
char PdfParser::getHexChar()
{
    char result = fromHex(getChar());
    return result * 16 + fromHex(getChar());
}

/**
 * Interpret the argument as a hexadecimal figure.
 * Throw a @ref ParseError if it isn't one.
 */
char PdfParser::fromHex(char c)
{
    c = tolower(c);
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else {
        std::stringstream ss; 
        ss << "expected a hex char, got '" << c << "'";
        throw ParseError(ss.str());
    }
}

/**
 * Checks if @arg c is a regular character.
 * PDF spec 7.2.2.
 */
bool PdfParser::isRegular(char c)
{
    return !isSpace(c) && !isDelimiter(c);
}

/**
 * Checks if @arg c is a delimiter.
 * PDF spec 7.2.2.
 */
bool PdfParser::isDelimiter(char c)
{
    switch (c) {
        case '(':
        case ')':
        case '<':
        case '>':
        case '[':
        case ']':
        case '{':
        case '}':
        case '/':
        case '%':
            return true;
        default:
            return false;
    }
}

/**
 * Parses a PDF number.
 * PDF spec 7.3.3.
 */
Pdf::Number *PdfParser::parseNumber()
{
    bool positive = true;
    int integral = 0, mantissa = 0;
    
    switch (getChar()) {
        case '+':
            break;
        case '-':
            positive = false;
        default:
            putChar();
    }
    
    try {
        integral = parseSimpleNumber();
    } catch (ParseError) {
        integral = 0;
    }
    
    if (getChar() == '.') try {
        mantissa = parseSimpleNumber();
    } catch (ParseError) {
        mantissa = 0;
    } else
        putChar();

    if (mantissa == 0)
        return new Pdf::Number((positive ? +1 : -1) * integral);
    else
        return new Pdf::Number((positive ? +1.0f : -1.0f) * (integral + 1.0f * mantissa / pow10(ceil(log10(mantissa)))));
}
