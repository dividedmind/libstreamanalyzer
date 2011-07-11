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

#include <iterator>
#include <boost/shared_ptr.hpp>
#include <strigi/streambase.h>

namespace Pdf {
//

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
    int64_t size() const;
    void seek(int64_t);
    char getChar();
    void putChar();

    class ConstIterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef char value_type;
        typedef int64_t difference_type;
        typedef const char *pointer;
        typedef const char &reference;
        ConstIterator(Parser *parent, int position);
        char operator++(int);
        char operator++();        
        char operator*() const;
        bool operator !=(const ConstIterator &other) const;
        bool operator ==(const ConstIterator &other) const;
        
    private:
        Parser *parent;
        int position;
    };
    ConstIterator here();
    ConstIterator end();

private:
    Strigi::StreamBase<char> *stream;
};

}

#endif // PDF_PARSER_H
