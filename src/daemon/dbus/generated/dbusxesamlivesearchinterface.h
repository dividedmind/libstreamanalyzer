// generated by makecode.pl
#ifndef DBUSXESAMLIVESEARCHINTERFACE_H
#define DBUSXESAMLIVESEARCHINTERFACE_H
#include "/home/oever/code/strigi/src/daemon/dbus/xesamlivesearchinterface.h"
#include <dbus/dbus.h>
class DBusObjectInterface;
class DBusXesamLiveSearchInterface : public XesamLiveSearchInterface {
private:
    DBusConnection* const conn;
    std::string object;
    DBusObjectInterface* const iface;
    void HitsModified(const std::string& search,         const std::vector<int32_t>& hit_ids);
    void HitsRemoved(const std::string& search,         const std::vector<int32_t>& hit_ids);
    void HitsAdded(const std::string& search, const int32_t count);
public:
    DBusXesamLiveSearchInterface(const std::string& objectname, DBusConnection* c);
    ~DBusXesamLiveSearchInterface();
    DBusObjectInterface* interface() { return iface; }
};
#endif