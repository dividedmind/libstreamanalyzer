/*
 *    Strigi PDF parser.
 *    Copyright (C) 2011  Rafał Rzepecki <divided.mind@gmail.com>
 * 
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 * 
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 * 
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <algorithm>

#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_domain.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_alternative.hpp>
#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_symbols.hpp>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_scope.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>

#include <iostream>

#include "xreftable.h"

BOOST_FUSION_ADAPT_STRUCT(Pdf::XRefTableIndex,
    (unsigned, number)
    (unsigned, generation)
)

BOOST_FUSION_ADAPT_STRUCT(Pdf::XRefTableEntry,
    (unsigned, offset)
    (bool, in_use)
)

namespace Pdf { 
namespace phx = boost::phoenix;

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream &o, const std::pair<T1, T2> &p)
{
    return o << "<" << p.first << ", " << p.second << ">";
}

std::ostream& operator<<(std::ostream& o, const XRefTable &t)
{
    using phx::lambda;
    using phx::val;
    using phx::at_c;
    
    using namespace phx::arg_names;
    
    o << "xref table, " << t.size() << " entries: " << std::endl;
    std::for_each(t.begin(), t.end(), o << _1 << std::endl);
    
    return o;
}


std::ostream& operator<<(std::ostream& o, const Pdf::XRefTableEntry& t)
{
    return o << "offset " << t.offset << (t.in_use ? " (in use)" : " (free)");
}
std::ostream& operator<<(std::ostream& o, const Pdf::XRefTableIndex& t)
{
    return o << '[' << t.number << ':' << t.generation << ']';
}

bool XRefTableIndex::operator<(const Pdf::XRefTableIndex& other) const
{
    return (number < other.number) || (number == other.number && generation < other.generation);
}

namespace Parser {
    namespace qi = boost::spirit::qi;
    using std::map;
    using std::pair;
    
    /* Cross-reference table parser. See PDF spec section 7.5.4. */
    struct xreftable_impl {
        xreftable_impl() {
            using qi::uint_;
            using namespace qi::labels;
            using qi::attr;
            using qi::omit;
            
            /* Two numbers in the header: number (index) of the first entry and number of entries. */
            xreftable %= "xref" >> 
                omit[uint_[_a = _1] >> // read the index of first entry into _a, we will use it to iterate
                uint_[_b = _1]] >> // _b is number of entries to read
                qi::repeat(_b)[xrefentry(_a)[_a++]]; // iterate
            
            xrefentry %= omit[uint_[_a = _1]] // extract offset
                >> index(_r1) // read generation and make index
                >> entry(_a);
            
            index %= attr(_r1) >> uint_;
            entry %= attr(_r1) >> in_use;
            
            in_use.add
                ("f", false)
                ("n", true)
            ;
            
            index.name("index");
            entry.name("entry");
            xrefentry.name("xrefentry");
            xreftable.name("xreftable");
        }

        qi::symbols<char, bool> in_use;

        // pass current index
        qi::rule<StreamWrapper::Iterator, simple_rule, 
            pair<XRefTableIndex, XRefTableEntry>(unsigned), qi::locals<unsigned> > xrefentry;

        // construction
        qi::rule<StreamWrapper::Iterator, simple_rule, XRefTableIndex(unsigned)> index;
        qi::rule<StreamWrapper::Iterator, simple_rule, XRefTableEntry(unsigned)> entry;
        
        /* the whole thing */
        qi::rule<StreamWrapper::Iterator, qi::locals<unsigned, unsigned>, simple_rule, XRefTable()> xreftable;
    };

    const xreftable_impl impl;
    xreftable_type xreftable = impl.xreftable;
}}
