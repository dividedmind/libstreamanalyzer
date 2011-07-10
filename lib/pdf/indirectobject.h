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

#ifndef PDF_INDIRECTOBJECT_H
#define PDF_INDIRECTOBJECT_H

#include <boost/shared_ptr.hpp>

#include "object.h"

using namespace boost;

namespace Pdf {
//

class IndirectObject : public Object {
public:
    IndirectObject(int number, int generation, Object *object) : number(number), generation(generation), object(object) {}
    int index() const { return number; }
    
private:
    virtual void pretty(std::ostream& stream) const;
    
    int number;
    int generation;
    shared_ptr<Object> object;
};

}

#endif // PDF_INDIRECTOBJECT_H
