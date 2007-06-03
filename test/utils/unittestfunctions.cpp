/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "unittestfunctions.h"

#include "indexmanager.h"
#include "combinedindexmanager.h"
#include <vector>

using namespace std;

Strigi::IndexManager* strigiunittest::getIndexManager(string& backend,
                                                      const string& indexdir)
{
    // check arguments: backend
    const vector<string>& backends = CombinedIndexManager::backEnds();

    vector<string>::const_iterator b
            = find(backends.begin(), backends.end(), backend);
    if (b == backends.end())
        return 0;

    return CombinedIndexManager::factories()[backend](indexdir.c_str());
}