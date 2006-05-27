#include "simplesearchgui.h"
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedWidget>
#include <QtGui/QLabel>
#include <QtCore/QProcess>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QProcess>
#include <QtGui/QPushButton>
#include <QtCore/QCoreApplication>
#include <string>
#include <vector>
#include "socketclient.h"
using namespace std;

SimpleSearchGui::SimpleSearchGui() {
    mainview = new QStackedWidget();
    itemview = new QListWidget();

    QWidget* statuswidget = new QWidget();
    QVBoxLayout *statuslayout = new QVBoxLayout;
    statusview = new QLabel();
    statusview->setAlignment(Qt::AlignTop);
    statusview->setMargin(25);
    indexing = false;
    running = false;
    starting = true;
    toggleindexing = new QPushButton("start indexing");
    toggledaemon = new QPushButton("stop daemon");
    statuslayout->addWidget(statusview);
    statuslayout->addWidget(toggleindexing);
    statuslayout->addWidget(toggledaemon);
    statuswidget->setLayout(statuslayout);

    mainview->addWidget(itemview);
    mainview->addWidget(statuswidget);
    mainview->setCurrentIndex(1);

    queryfield = new QLineEdit();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainview);
    layout->addWidget(queryfield);
    setLayout(layout);

    connect(queryfield, SIGNAL(textChanged(const QString&)),
        this, SLOT(query(const QString&)));
    connect(&executer, SIGNAL(queryFinished(const QString&)),
        this, SLOT(handleQueryResult(const QString&)));
    connect(itemview, SIGNAL(itemClicked(QListWidgetItem*)),
        this, SLOT(openItem(QListWidgetItem*)));
    connect(toggleindexing, SIGNAL(clicked()),
        this, SLOT(toggleIndexing()));
    connect(toggledaemon, SIGNAL(clicked()),
        this, SLOT(toggleDaemon()));
    itemview->setEnabled(false);
    queryfield->setFocus(Qt::ActiveWindowFocusReason);

    socketfile = getenv("HOME");
    socketfile += "/.kitten/socket";

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    timer->start(2000);
    updateStatus();
}
void
SimpleSearchGui::query(const QString& item) {
    QString query = item.trimmed();
    if (query.length() == 0) {
        mainview->setCurrentIndex(1);
    } else {
        mainview->setCurrentIndex(0);
        itemview->setEnabled(false);
        itemview->clear();
        itemview->addItem("searching...");
        executer.query(query);
    }
}
void
SimpleSearchGui::updateStatus() {
    static bool first = true;
    static bool attemptedstart = false;
    if (!first && !statusview->isVisible()) return;
    first = false;
    SocketClient client;
    client.setSocketName(socketfile.c_str());
    map<string,string> s = client.getStatus();
    if (s.size() == 0) {
        running = false;
        if (!attemptedstart) {
            s["Status"] = "Starting daemon";
            startDaemon();
            attemptedstart = true;
        } else {
            s["Status"] = "Daemon is not running";
        }
    } else {
        attemptedstart = true;
        starting = false;
        running = true;
    }
    toggleindexing->setEnabled(running);
    queryfield->setEnabled(running);
    queryfield->setEnabled(!starting);
    toggledaemon->setText((running)?"stop daemon":"start daemon");
    bool idxng = s["Status"] == "indexing";
    if (idxng != indexing) {
        indexing = idxng;
        toggleindexing->setText((indexing)?"stop indexing":"start indexing");
    }
    
    map<string,string>::const_iterator i;
    QString text;
    for (i = s.begin(); i != s.end(); ++i) {
        text += i->first.c_str();
        text += ":\t";
        text += i->second.c_str();
        text += "\n";
    }
    statusview->setText(text);
}
void
SimpleSearchGui::startDaemon() {
    starting = true;
    // try to start the daemon
    QFileInfo exe = QCoreApplication::applicationDirPath()
	+ "/../../daemon/kittendaemon";
    if (exe.exists()) {
	// start not installed version
	QProcess::startDetached(exe.absoluteFilePath());
    } else {
        exe = QCoreApplication::applicationDirPath()+"/kittendaemon";
        if (exe.exists()) {
            QProcess::startDetached(exe.absoluteFilePath());
        } else {
	    // start installed version
	    QProcess::startDetached("kittendaemon");
        }
    }
}
void
SimpleSearchGui::handleQueryResult(const QString& item) {
    itemview->clear();
    vector<string> results = executer.getResults();

    if (results.size() > 0) {
        if (results[0] != "error") {
            itemview->setEnabled(true);
        }
        for (uint i=0; i<results.size(); ++i) {
            itemview->addItem(results[i].c_str());
        }
    } else {
        itemview->addItem("no results");
    }
}
void
SimpleSearchGui::openItem(QListWidgetItem* i) {
    // if the file does not exist on the file system remove items of the end
    // until it does
    QString file = i->text();
    QFileInfo info(file);
    while (!info.exists()) {
        int pos = file.lastIndexOf('/');
        if (pos <= 0) return;
        file = file.left(pos);
        info.setFile(file);
    }
    if (file.endsWith(".tar") || file.endsWith(".tar.bz2")
            || file.endsWith(".tar.gz")) {
        file = "tar:"+i->text();
    } else if (file.endsWith(".zip") || file.endsWith(".jar")) {
        file = "zip:"+i->text();
    }
    QStringList args;
    args << "openURL" << file;
    QProcess::execute("kfmclient", args);
    qDebug() << i->text();
}
void
SimpleSearchGui::toggleDaemon() {
    if (running) {
        SocketClient client;
        client.setSocketName(socketfile.c_str());
        client.stopDaemon();
    } else {
        startDaemon();
    }
}
void
SimpleSearchGui::toggleIndexing() {
    SocketClient client;
    client.setSocketName(socketfile.c_str());
    if (indexing) {
        client.stopIndexing();
    } else {
        client.startIndexing();
    }
}
