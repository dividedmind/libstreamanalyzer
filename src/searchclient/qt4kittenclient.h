#ifndef QT4KITTENCLIENT_H
#define QT4KITTENCLIENT_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include "asyncsocketclient.h"

class Qt4KittenClient : public QObject {
Q_OBJECT
private:
    enum Mode { Idle, CountHits, Query };
    Mode mode;
    AsyncSocketClient socket;
    QTimer poller;
    QQueue<QString> countQueue;
    QQueue<QString> queryQueue;

    ClientInterface::Hits hits;

    void startCountHits();
    void startQuery();
public:
    Qt4KittenClient();
private slots:
    void poll();
public slots:
    void countHits(const QString&);
    void query(const QString&);
signals:
    void gotHitsCount(const QString& query, int count);
    void gotHits(const QString&, const ClientInterface::Hits&);
};

#endif
