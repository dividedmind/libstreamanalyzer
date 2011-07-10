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


#ifndef PDF_STREAM_H
#define PDF_STREAM_H

#include <boost/shared_ptr.hpp>
#include <strigi/streambase.h>

#include "object.h"
#include "dictionary.h"

namespace Pdf {
    

class Stream : public Pdf::Object
{
public:
    Stream(Dictionary *dict, Strigi::StreamBase<char> *stream) : dict(dict), stream(stream) {}
    
private:
    boost::shared_ptr<Dictionary> dict;
    boost::shared_ptr< Strigi::StreamBase<char> > stream;
    
    virtual void pretty(std::ostream& stream) const;
};

}

#endif // PDF_STREAM_H
