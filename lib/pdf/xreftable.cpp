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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <iostream>
#include <map>
#include <utility>

#include "xreftable.h"

namespace Pdf { namespace Parser {
    namespace qi = boost::spirit::qi;
    namespace phx = boost::phoenix;
    using boost::tuple;
    using boost::optional;
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
            xreftable = "xref" >> 
                omit[uint_[_a = _1] >> // read the index of first entry into _a, we will use it to iterate
                uint_[_b = _1]] >> // _b is number of entries to read
                qi::repeat(_b)[xrefentry(_a)[_a++]]; // iterate
            
            xrefentry = active_entry(_r1) | inactive_entry;
            /* file offset, generation and usage */
            inactive_entry = uint_ >> uint_ >> 'f'; // ignore inactive entries
            /* (we need some rearranging) */
            active_entry = omit[uint_[_a = _1]] // extract offset
                >> index(_r1) // read generation and make index
                >> 'n' >> attr(_a); // return offset
            
            index = attr(_r1) >> uint_;
            
            index.name("index");
            xrefentry.name("xrefentry");
            xreftable.name("xreftable");
        }
        
        typedef pair<unsigned, unsigned> xreftable_index;
        typedef pair<xreftable_index, unsigned> entry;
        
        /* index, generation and offset */
        // pass current index
        qi::rule<StreamWrapper::Iterator, simple_rule, optional<entry>(unsigned)> xrefentry;
        qi::rule<StreamWrapper::Iterator, simple_rule, entry(unsigned), qi::locals<unsigned> > active_entry;
        qi::rule<StreamWrapper::Iterator, simple_rule> inactive_entry;

        // index construction
        qi::rule<StreamWrapper::Iterator, simple_rule, xreftable_index(unsigned)> index;
        
        /* the whole thing */
        qi::rule<StreamWrapper::Iterator, qi::locals<unsigned, unsigned>, simple_rule, map<xreftable_index, unsigned>()> xreftable;
    };

    const xreftable_impl impl;
    const skip_rule xreftable = impl.xreftable;
}}
