/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Jos van den Oever <jos@vandenoever.info>
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
#ifndef XESAMLIVESEARCHINTERFACE_H
#define XESAMLIVESEARCHINTERFACE_H

#include "variant.h"
#include <map>

class XesamLiveSearchInterface {
private:
    XesamLiveSearchInterface* const iface;
public:
    XesamLiveSearchInterface(XesamLiveSearchInterface* x) :iface(x) {}
    virtual ~XesamLiveSearchInterface() {}
    virtual std::string NewSession() {
        return iface->NewSession();
    }
    virtual Variant SetProperty(const std::string& session,
            const std::string& prop, const Variant& v) {
        return iface->SetProperty(session, prop, v);
    }
    virtual void CloseSession(const std::string& session) {
        iface->CloseSession(session);
    }
    virtual std::string NewSearch(const std::string& session,
            const std::string& query_xml) {
        return iface->NewSearch(session, query_xml);
    }
    virtual int32_t CountHits(const std::string& search) {
        return iface->CountHits(search);
    }
    virtual std::vector<std::vector<Variant> > GetHits(
            const std::string& search, int32_t num) {
        return iface->GetHits(search, num);
    }
    virtual std::vector<std::vector<Variant> > GetHitData(
            const std::string& search,
            const std::vector<int32_t>& hit_ids,
            const std::vector<std::string>& properties) {
        return iface->GetHitData(search, hit_ids, properties);
    }
    virtual void CloseSearch(const std::string& search) {
        return iface->CloseSearch(search);
    }
    virtual std::vector<std::string> GetState() {
        return iface->GetState();
    }

    virtual void HitsAdded(const std::string& search, const int32_t count) = 0; 
    virtual void HitsRemoved(const std::string& search,
        const std::vector<int32_t>& hit_ids) = 0;
    virtual void HitsModified(const std::string& search,
        const std::vector<int32_t>& hit_ids) = 0;
};

#endif