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


#ifndef PDF_DICTIONARY_H
#define PDF_DICTIONARY_H

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/proto/proto_fwd.hpp> // old spirit versions miss this include
#include <boost/spirit/include/qi_rule.hpp>

#include "object.h"
#include "parser.h"
#include "grammar.h"

namespace Pdf {

class Dictionary : public Pdf::Object, public std::map< std::string, boost::shared_ptr<Pdf::Object> >
{
public:
    const Pdf::Object &get(const std::string &key) const;
    static const boost::spirit::qi::rule<Parser::ConstIterator, Grammar::whitespace_type> parser;

private:
    virtual void pretty(std::ostream& stream) const;
};

}

#endif // PDF_DICTIONARY_H
