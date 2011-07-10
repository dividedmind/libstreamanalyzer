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


#ifndef PDF_NUMBER_H
#define PDF_NUMBER_H

#include "object.h"


namespace Pdf {

class Number : public Pdf::Object
{
public:
    Number(int num);
    Number(double num);
    
    operator int() const;
    
private:
    enum Type {
        Int, Real
    };
    Type type;
    int intValue;
    double realValue;
    
    virtual void pretty(std::ostream& stream) const;
};

}

#endif // PDF_NUMBER_H
