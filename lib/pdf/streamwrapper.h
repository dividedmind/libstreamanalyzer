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

#ifndef PDF_STREAMWRAPPER_H
#define PDF_STREAMWRAPPER_H

#include <iterator>
#include <strigi/streambase.h>

namespace Pdf {
//

class StreamWrapper {
public:
    StreamWrapper(Strigi::StreamBase<char> *stream);
    int64_t size() const;
    void seek(int64_t);
    char getChar();
    void putChar();

    class Iterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef char value_type;
        typedef int64_t difference_type;
        typedef const char *pointer;
        typedef const char &reference;
        Iterator(StreamWrapper *parent, int position);
        Iterator operator++(int);
        Iterator &operator++();        
        char operator*() const;
        bool operator !=(const Iterator &other) const;
        bool operator ==(const Iterator &other) const;
        
    private:
        StreamWrapper *parent;
        int position;
    };
    Iterator begin() { return Iterator(this, 0); }
    Iterator here();
    Iterator end();

private:
    Strigi::StreamBase<char> *stream;
};

}

#endif // PDF_STREAMWRAPPER_H
