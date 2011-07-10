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
#include <strigi/subinputstream.h>
#include <strigi/kmpsearcher.h>

#include "dictionary.h"
#include "name.h"
#include "number.h"
#include "stream.h"
#include "reference.h"
#include "array.h"
#include "string.h"
#include "xreftable.h"

#include "pdfparser.h"

#define forever for(;;)

PdfParser::PdfParser() :streamhandler(0), texthandler(0), 
buffer(0), pos(0), bufEnd(0), bufferStart(0) {
}

Strigi::StreamStatus
PdfParser::parse(Strigi::StreamBase<char>* stream) throw (ParseError) {
    this->stream = stream;

    resetStream(stream->size());
    findBackwards("startxref");
    checkKeyword("startxref");
    startXRef = parseSimpleNumber();
    
    findBackwards("trailer");
    checkKeyword("trailer");
    skipWhitespaceAndComments();
    if (getChar() != '<' || getChar() != '<')
        throw ParseError("expected trailer dictionary");
    
    
    trailer = boost::shared_ptr<Pdf::Dictionary>(parseDictionary());
    if (trailer->count("Prev"))
        std::cerr << "PdfParser warning: multiple xref tables not yet supported" << std::endl;
    
    int size = dynamic_cast<const Pdf::Number &>(trailer->get("Size"));
    objects.resize(size);
    xRefTable = boost::shared_ptr<Pdf::XRefTable>(new Pdf::XRefTable(size));
    resetStream(startXRef);
    xRefTable->parse(this);

    boost::shared_ptr<Pdf::Object> root = trailer->at("Root");
    if (Pdf::Reference *ref = dynamic_cast<Pdf::Reference *>(root.get()))
        root = dereference(ref);

    std::cerr << *root;
    
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
    if (bufEnd - pos >= minChars)
        return;
    if (bufEnd != pos)
        stream->reset(currentPosition());

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
    unsigned int objectNumber = parseSimpleNumber();
    unsigned int generationNumber = parseSimpleNumber();
    checkKeyword("obj");
    
    Pdf::Object *object = parseObject();
    
    checkKeyword("endobj");
    
    if (objects.size() <= objectNumber)
        objects.resize(objectNumber + 1);
    else if (objects[objectNumber])
        throw ParseError("duplicate object number"); // TODO handle multiple generations
    
    objects[objectNumber] = boost::shared_ptr<Pdf::Object>(object);
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
            if (getChar() == '<') {
                Pdf::Dictionary *dict = parseDictionary();
                if (dict->count("Length") > 0)
                    return parseStream(dict);
                else
                    return dict;
            } else
                return parseHexString();
        default:
            throw ParseError("unknown object type");
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
            return parseNumberOrReference();
        case '[':
            return parseArray();
        case '(':
            return parseLiteralString();
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
        (*dict)[*name] = boost::shared_ptr<Pdf::Object>(value);
        delete name;
        
        skipWhitespaceAndComments();
        if (getChar() == '>') {
            if (getChar() == '>')
                return dict;
            else putChar();
        } else putChar();
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
    
    if (result == -1) {
        putChar();
        throw ParseError("expected hexadecimal number");
    }
    
    char c = fromHex(getChar());
    if (c == -1) {
        putChar();
        throw ParseError("expected hexadecimal number");
    }
    
    return result * 16 + c;
}

/**
 * Interpret the argument as a hexadecimal figure.
 * Return -1 if it isn't one.
 */
char PdfParser::fromHex(char c)
{
    c = tolower(c);
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else 
        return -1;
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
        case '-':
            positive = false;
        case '+':
            break;
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

/** Isolates a PDF stream.
 * PDF spec section 7.3.8.
 * @arg dict is the stream dictionary.
 */
Pdf::Stream* PdfParser::parseStream(Pdf::Dictionary* dict)
{
    checkKeyword("stream");
    
    char c = getChar();
    if (c == '\r')
        c = getChar();
    if (c != '\n')
        throw ParseError("EOL expected after 'stream' keyword");

    stream->reset(currentPosition());
    bufEnd = pos;
    
    Pdf::Number *length = dynamic_cast<Pdf::Number *>((*dict)["Length"].get());
    if (!length)
        throw ParseError("invalid stream length (not a number)");
    
    Strigi::StreamBase<char> *subStream = new Strigi::SubInputStream(stream, *length);
    Pdf::Stream *streamObject = new Pdf::Stream(dict, subStream);
    
    stream->skip(*length);
    bufferStart = stream->position();
    bufEnd = pos = buffer;
    
    checkKeyword("endstream");
    
    return streamObject;
}

/**
 * @return the current position in the stream.
 */
int64_t PdfParser::currentPosition()
{
    return bufferStart + (pos - buffer);
}

/**
 * Parse a number or reference object
 */
Pdf::Object *PdfParser::parseNumberOrReference()
{
    Pdf::Number *number = parseNumber();
    
    int64_t position = currentPosition();
    
    Pdf::Number *generation = 0;
    try {
        generation = parseNumber();
        checkKeyword("R");
        Pdf::Reference *ref = new Pdf::Reference(*number, *generation);
        delete number;
        delete generation;
        return ref;
    } catch (ParseError) {
        resetStream(position);
        return number;
    }
}

/**
 * Set stream position.
 */
void PdfParser::resetStream(int64_t position)
{
    pos = buffer + position - bufferStart;
    if (position < bufferStart) {
        stream->reset(position);
        bufferStart = stream->position();
        bufEnd = pos = buffer;
    } else if (pos > bufEnd) {
        bufferStart = bufEnd - buffer + bufferStart;
        stream->reset(bufferStart);
        stream->skip(pos - bufEnd);
        bufferStart += pos - bufEnd;
        bufEnd = pos = buffer;
    }
}

/**
 * Parse an array object.
 * PDF spec section 7.3.6.
 */
Pdf::Array *PdfParser::parseArray()
{
    Pdf::Array *arr = new Pdf::Array;
    
    skipWhitespaceAndComments();
    
    while (getChar() != ']') {
        putChar();
        arr->push_back(boost::shared_ptr<Pdf::Object>(parseObject()));
        skipWhitespaceAndComments();
    }
    
    return arr;
}

/**
 * Parse a literal string.
 * PDF spec section 7.3.4.2.
 */
Pdf::String *PdfParser::parseLiteralString()
{
    int parentheses = 0;
    char c;
    std::string str;
    
    forever switch (c = getChar()) {
        case '(':
            parentheses += 1;
            break;
        case ')':
            if (parentheses == 0)
                return new Pdf::String(str);
            else parentheses--;
            break;
        case '\\':
            switch (getChar()) {
                case 'n':
                    str += '\n';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case 't':
                    str += '\t';
                    break;
                case 'b':
                    str += '\b';
                    break;
                case 'f':
                    str += '\f';
                    break;
                case '(':
                    str += '(';
                    break;
                case ')':
                    str += ')';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    putChar();
                    str += getOctalChar();
                    break;
                case '\n':
                    if (getChar() != '\r')
                        putChar();
                    break;
                case '\r':
                    if (getChar() != '\n')
                        putChar();
                    break;
                default:
                    putChar();
            }
            break;
        default:
            str += c;
    }
}

/**
 * Get an octal number. 
 * It has to have at least one and at most three digits. 
 * It may be zero-padded.
 */
char PdfParser::getOctalChar()
{
    char c, result = 0, count = 0;
    while (count++ < 3) switch (c = getChar()) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            result *= 8;
            result += c - '0';
        default:
            putChar();
            return result;
    }
    
    return result;
}

/** 
 * Parse a hex string.
 * PDF spec section 7.3.4.3.
 */
Pdf::String *PdfParser::parseHexString()
{
    std::string str;
    
    forever {
        char c = getChar();
        if (c == '>')
            return new Pdf::String(str);
        
        char current = fromHex(c);
        if (current < 0) {
            putChar();
            throw ParseError("expected hexadecimal number");
        }
        
        current *= 16;
        
        c = getChar();
        
        if (c == '>')
            return new Pdf::String(str += c);
        
        char r = fromHex(c);
        if (r < 0) {
            putChar();
            throw ParseError("expected hexadecimal number");
        }
        str += current + r;
    }
}

/**
 * Find a text backwards from the current position.
 */
void PdfParser::findBackwards(const char *text)
{
    int wrap = 4;
    int64_t startPosition = currentPosition();
    
    Strigi::KmpSearcher searcher(text);

    forever {
        resetStream(startPosition - wrap);
        fillBuffer(wrap);
        const char *final = 0;
        const char *result = buffer - 1;
        while ((result = searcher.search(result + 1, bufEnd - result - 1)))
            final = result;
        if (final) {
            pos = bufEnd = buffer;
            resetStream(bufferStart + final - buffer);
            return;
        } else {
            wrap *= 2;
            if (wrap > startPosition) {
                wrap = startPosition;
            } else if (wrap == startPosition)
                throw ParseError("couldn't find keyword");
        }
    }
}

boost::shared_ptr< Pdf::Object > PdfParser::dereference(Pdf::Reference* ref)
{
    if (!objects[ref->index()]) {
        resetStream(xRefTable->at(ref->index()).second);
        (void) parseIndirectObject();
    }
    
    return objects[ref->index()];
}
