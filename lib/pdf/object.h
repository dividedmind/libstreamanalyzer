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

#ifndef PDF_OBJECT_H
#define PDF_OBJECT_H
#include <iostream>

namespace Pdf {

class Object
{
public:
    virtual ~Object() {}

protected:
    virtual void pretty(std::ostream &stream) const = 0;
    
    friend std::ostream &operator <<(std::ostream &stream, const Object &object);
};

std::ostream &operator <<(std::ostream &stream, const Pdf::Object &object);
}

#endif // PDF_OBJECT_H
