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

#include "streamwrapper.h"
#include "footer.h"

#include "pdfparser.h"

PdfParser::PdfParser() :streamhandler(0), texthandler(0)
{
}

Strigi::StreamStatus
PdfParser::parse(Strigi::StreamBase<char>* stream) {
    using Pdf::StreamWrapper;
    Pdf::StreamWrapper wrapper(stream);
    
    wrapper.seek(wrapper.size() - 512);
    for (StreamWrapper::Iterator it = wrapper.here(); it != wrapper.end(); ++it) {
        StreamWrapper::Iterator current(it);
        if (Pdf::Footer::parse(current, wrapper.end()))
            return stream->status();
    }
    
    throw std::string("couldn't find footer");
}
