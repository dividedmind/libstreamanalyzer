/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -p strigidbus -i strigitypes.h strigiinterface.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech ASA. All rights reserved.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef STRIGIDBUS_H_1165266067
#define STRIGIDBUS_H_1165266067

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "strigitypes.h"

/*
 * Proxy class for interface vandenoever.strigi
 */
class VandenoeverStrigiInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "vandenoever.strigi"; }

public:
    VandenoeverStrigiInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~VandenoeverStrigiInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<int> countHits(const QString &query)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(query);
        return callWithArgumentList(QDBus::Block, QLatin1String("countHits"), argumentList);
    }

    inline QDBusReply<QStringList> getBackEnds()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getBackEnds"), argumentList);
    }

    inline QDBusReply<QStringList> getFieldNames()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getFieldNames"), argumentList);
    }

    inline QDBusReply<QList<QPair<bool,QString> > > getFilters()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getFilters"), argumentList);
    }

    inline QDBusReply<QList<QPair<QString,quint32> > > getHistogram(const QString &query, const QString &field, const QString &labeltype)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(query) << qVariantFromValue(field) << qVariantFromValue(labeltype);
        return callWithArgumentList(QDBus::Block, QLatin1String("getHistogram"), argumentList);
    }

    inline QDBusReply<QList<StrigiHit> > getHits(const QString &query, uint max, uint offset)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(query) << qVariantFromValue(max) << qVariantFromValue(offset);
        return callWithArgumentList(QDBus::Block, QLatin1String("getHits"), argumentList);
    }

    inline QDBusReply<QStringList> getIndexedDirectories()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getIndexedDirectories"), argumentList);
    }

    inline QDBusReply<QStringList> getIndexedFiles()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getIndexedFiles"), argumentList);
    }

    inline QDBusReply<QMap<QString,QString> > getStatus()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getStatus"), argumentList);
    }

    inline QDBusReply<void> indexFile(const QString &path, qulonglong mtime, const QByteArray &content)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(path) << qVariantFromValue(mtime) << qVariantFromValue(content);
        return callWithArgumentList(QDBus::Block, QLatin1String("indexFile"), argumentList);
    }

    inline QDBusReply<bool> isActive()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("isActive"), argumentList);
    }

    inline QDBusReply<void> setFilters(const QList<QPair<bool,QString> > &rules)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(rules);
        return callWithArgumentList(QDBus::Block, QLatin1String("setFilters"), argumentList);
    }

    inline QDBusReply<QString> setIndexedDirectories(const QStringList &d)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(d);
        return callWithArgumentList(QDBus::Block, QLatin1String("setIndexedDirectories"), argumentList);
    }

    inline QDBusReply<QString> startIndexing()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("startIndexing"), argumentList);
    }

    inline QDBusReply<QString> stopDaemon()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("stopDaemon"), argumentList);
    }

    inline QDBusReply<QString> stopIndexing()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("stopIndexing"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace vandenoever {
  typedef ::VandenoeverStrigiInterface strigi;
}
#endif
