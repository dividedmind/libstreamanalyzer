#include "strigihtmlgui.h"
#include "socketclient.h"
#include "indexreader.h"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sstream>
using namespace std;
using namespace jstreams;

class StrigiHtmlGui::Private {
private:
    HtmlHelper* h;
    string highlightTerms(const string& t, const Query& q,
        const vector<string>& fields) const;
    void printSearchResult(ostream& out,
        const jstreams::IndexedDocument& doc, const Query& q) const;
public:
    SocketClient strigi;

    Private(HtmlHelper* h);
    void printSearchResults(ostream& out, const ClientInterface::Hits&,
        const string& query) const;
};

StrigiHtmlGui::StrigiHtmlGui(HtmlHelper* h) : helper(h) {
    p = new Private(helper);
}
StrigiHtmlGui::~StrigiHtmlGui() {
    delete p;
}
void
StrigiHtmlGui::printHeader(ostream& out, const string& path,
        const map<string, string> &params) {
    out << "<?xml version='1.0' encoding='utf-8'?>\n"
        "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' "
        "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n"
        "<html xmlns='http://www.w3.org/1999/xhtml'>"
        "<head><meta http-equiv='Content-Type' "
        "content='text/html; charset=utf-8' />"
        "<link rel='stylesheet' type='text/css' href='"
        << helper->getCssUrl()
        << "'/>"
        "<title>Strigi Desktop Search</title>"
        "</head><body>"
        "<div class='header'>";
    printMenu(out, path, params);
    out << "<div class='title'>Strigi Desktop Search</div>";
    out << "</div><div class='box'>";
}
void
StrigiHtmlGui::printFooter(ostream& out, const string& path,
        const map<string, string> &params) {
    out << "</div>";
    out << "</body></html>";
}
void
StrigiHtmlGui::printHelp(ostream& out, const string& path,
        const map<string, string> &params) {
    out << "For help see the <a href='http://strigi.sf.net'>Strigi Wiki</a>";
}
void
StrigiHtmlGui::printAbout(ostream& out, const string& path,
        const map<string, string> &params) {
    out << "<a href='http://www.vandenoever.info/software/strigi'>The Strigi Website</a>";
}
void
StrigiHtmlGui::printConfig(ostream& out, const string& path,
        const map<string, string> &params) {
    printIndexedDirs(out, path, params);
}
void
startDaemon() {
    if (fork()) {
        char * const args[] = {"strigidaemon", "clucene", 0};
        execvp("/home/oever/testinstall/bin/strigidaemon", args);
    }
}
void
StrigiHtmlGui::printStatus(ostream& out, const string& path,
        const map<string, string> &params) {
    map<string, string> status;
    if (path == "status/start") {
        status = p->strigi.getStatus();
        if (status.size() == 0) {
            startDaemon();
            int n = 0;
            while (n < 5 && status.size() == 0) {
                sleep(1);
                status = p->strigi.getStatus();
                n++;
            }
        }
    } else if (path == "status/stop") {
        p->strigi.stopDaemon();
    } else if (path == "status/stopindexing") {
        p->strigi.stopIndexing();
        status = p->strigi.getStatus();
    } else if (path == "status/startindexing") {
        p->strigi.startIndexing();
        status = p->strigi.getStatus();
    } else {
        status = p->strigi.getStatus();
    }
    if (status.size() == 0) {
        out << "<p><a href='/status/start'>Start daemon</a></p>";
    } else {
        map<string, string>::const_iterator i;
        out << "<table>";
        for (i = status.begin(); i != status.end(); ++i) {
            out << "<tr><td>" << i->first << "</td><td>" << i->second
                << "</td><tr>";
        }
        out << "</table>";
        out << "<p><a href='/status/stop'>Stop daemon</a></p>";
        if (status["Status"] == "indexing") {
            out << "<p><a href='/status/stopindexing'>Stop indexing</a></p>";
        } else {
            out << "<p><a href='/status/startindexing'>Start indexing</a></p>";
        }
    }
    // automatically reload the status page
    out << "<script type='text/javascript'>\n"
        "setTimeout('window.location.replace(\"/status\")', 2000);\n"
        "</script>\n";
}
void
StrigiHtmlGui::printSearch(ostream& out, const string& path,
        const map<string, string> &params) {
    string query;
    map<string, string>::const_iterator i = params.find("q");
    if (i != params.end()) query = i->second;
    int max = 10;
    i = params.find("m");
    if (i != params.end()) {
        max = atoi(i->second.c_str());
    }
    if (max == 0) max = 10;
    int off = 0;
    i = params.find("o");
    if (i != params.end()) {
        off = atoi(i->second.c_str());
    }
    string selectedtab;
    i = params.find("t");
    if (i != params.end()) {
        selectedtab = i->second;
    }

    // print tabs
    int count = 0;
    if (query.length()) {
        count = p->strigi.countHits(query);
    }
    string activetab;
    string activequery = query;
    map<string, int> hitcounts;
    if (count) {
        map<string, string> tabs;
        bool doother = true;
        tabs["Images"] = "mimetype:image*";
        tabs["Mail"] = "mimetype:message/*";
        tabs["Web"] = "mimetype:text/html";
        tabs["Text"] = "mimetype:text/*";
        map<string, string>::const_iterator j;
        string otherq = query;
        for (j = tabs.begin(); j != tabs.end(); ++j) {
            string q = query+" "+j->second;
            int c = p->strigi.countHits(q);
            if (c > 0) {
                hitcounts[j->first] = c;
                doother &= c < count;
		otherq += " -" + j->second;
                if (j->first == selectedtab || activetab.size() == 0) {
                    activetab = j->first;
                    activequery = q;
                }
            }
        }
        doother &= hitcounts.size() > 0;
        int othercount = 0;
        if (doother) {
            othercount = p->strigi.countHits(otherq);
            if (othercount > 0) {
                hitcounts["Other"] = othercount;
                if (selectedtab == "Other") {
                    activetab = selectedtab;
                    activequery = otherq;
                }
            }
        }
    }

    // print gui
    out << "<div class='control' style='text-align:right;' padding='5px'>";
    out << "<form method='get'>";
    out << "<input type='text' name='q' value='" << query << "'/>";
    out << "<input type='hidden' name='o' value='" << off << "'/>";
    out << "<input type='hidden' name='m' value='" << max << "'/>";
    out << "<input type='hidden' name='t' value='" << activetab << "'/>";
    out << "<input type='submit' value='search'/>";
    if (hitcounts.size() == 0 && count > 0) {
        out << " Found " << count << " results.";
    } else {
        map<string, int>::const_iterator l;
        for (l = hitcounts.begin(); l != hitcounts.end(); ++l) {
            if (l->first == activetab) {
                out << " <a class='activetab' ";
            } else {
                out << " <a class='tab' ";
            }
            out << "href='?q=" << query << "&t=" << l->first << "'>"
                << l->first << "&nbsp;(" << l->second << ")" << "</a>";
        }
    }
    out << "</form></div>\n";

    if (activequery.length()) {
        out << "<div class='hits'>";
        const ClientInterface::Hits hits = p->strigi.getHits(activequery,
            max, off);
        p->printSearchResults(out, hits, activequery);
        out << "</div>";
    }
}
string
StrigiHtmlGui::Private::highlightTerms(const string& t, const Query& q,
        const vector<string>& fields) const {
    string out = t;
    vector<string> terms;
    vector<string>::const_iterator i;
    for (i = fields.begin(); i != fields.end(); ++i) {
        map<string, set<string> >::const_iterator j = q.getIncludes().find(*i);
        if (j != q.getIncludes().end()) {
            set<string>::const_iterator k;
            for (k = j->second.begin(); k != j->second.end(); ++k) {
                terms.push_back(*k);
            }
        }
    }
    return h->highlight(out, terms);
}
void
StrigiHtmlGui::printMenu(ostream& out, const string& path,
        const map<string, string> &params) {
    out << "<div class='menu'>" << endl;
    out << "<a href='/'>search</a> " << endl;
    out << "<a href='/status'>status</a> " << endl;
    out << "<a href='/config'>preferences</a> " << endl;
    out << "<a href='/help'>help</a> " << endl;
    out << "<a href='/about'>about</a> " << endl;
    out << "</div>" << endl;
}
void
StrigiHtmlGui::printIndexedDirs(ostream& out, const string& path,
        const map<string, string> &params) {
    set<string> dirs = p->strigi.getIndexedDirectories();
    map<string,string>::const_iterator i = params.find("adddir");
    if (i != params.end()) {
        DIR* dir = opendir(i->second.c_str());
        if (dir) {
            dirs.insert(i->second);
            closedir(dir);
            p->strigi.setIndexedDirectories(dirs);
            out << "<p>Directory added. Don't forget to start indexing.</p>";
        }
    }
    i = params.find("deldir");
    if (i != params.end()) {
        uint oldsize = dirs.size();
        dirs.erase(i->second);
        if (dirs.size() != oldsize) {
            p->strigi.setIndexedDirectories(dirs);
        }
    }

    out << "<table>";
    set<string>::const_iterator j;
    for (j = dirs.begin(); j != dirs.end(); ++j) {
        out << "<tr><td><form method='get'>"
            "<input type='hidden' name='deldir' value='" << *j << "'/>"
            "<input type='submit' value='delete directory'/></form></td><td>"
            << *j << "</td></tr>";
    }
    out << "<form><tr><td><input type='submit' value='add directory'/></td>"
        "<td><input name='adddir' type='file'/></td></tr></form>";
    out << "</table>";
}
void
StrigiHtmlGui::printPage(ostream& out, const string& path,
        const map<string, string> &params) {
    printHeader(out, path, params);

    bool running = p->strigi.getStatus().size() > 0;
    if (strncmp(path.c_str(), "help", 4) == 0) {
        printHelp(out, path, params);
    } else if (strncmp(path.c_str(), "about", 5) == 0) {
        printAbout(out, path, params);
    } else if (running && strncmp(path.c_str(), "config", 6) == 0) {
        printConfig(out, path, params);
    } else if (strncmp(path.c_str(), "status", 6) == 0) {
        printStatus(out, path, params);
    } else if (running) {
        printSearch(out, path, params);
    } else {
        printStatus(out, path, params);
    }

    printFooter(out, path, params);
}
StrigiHtmlGui::Private::Private(HtmlHelper* helper) :h(helper) {
    string homedir = getenv("HOME");
    strigi.setSocketName(homedir+"/.strigi/socket");
}
string
toSizeString(int s) {
    ostringstream o;
    if (s > 1024) {
        o << (s+512)/1024 << "k";
    } else {
        o << s << " bytes";
    }
    return o.str();
}
void
StrigiHtmlGui::Private::printSearchResult(ostream& out,
        const jstreams::IndexedDocument& doc, const Query& query) const {
    string link, icon, name, folder;
    link = h->mapLinkUrl(doc.uri);
    icon = "<div class='iconbox'><img class='icon' src='";
    icon += h->mapMimetypeIcon(doc.uri, doc.mimetype);
    icon += "'/></div>\n";
    map<string, string>::const_iterator t = doc.properties.find("title");
    size_t l = doc.uri.rfind('/');
    if (t != doc.properties.end()) {
        name = t->second.c_str();
    } else if (l != string::npos) {
        name = doc.uri.substr(l+1);
    } else {
        name = doc.uri;
    }
    if (l != string::npos) {
        folder = doc.uri.substr(0, l);
    } 
    out << "<div class='hit'>" << icon << "<h2><a href='" << link << "'>";
    out << name << "</a></h2>";
/*    out << "<br/>score: ";
    out << doc.score << ", mime-type: " << doc.mimetype.c_str() << ", size: ";
    out << doc.size << ", last modified: " << h->formatDate(doc.mtime);*/
    string fragment = h->escapeString(doc.fragment);
    vector<string> fields;
    fields.push_back("");
    fields.push_back("content");
    fragment = highlightTerms(fragment, query, fields);
    out << "<div class='fragment'>" << fragment << "</div>";
    fields.clear();
    string path = h->escapeString(doc.uri);
    path = highlightTerms(path, query, fields);
    out << "<div class='path'><a href='" << link << "'>" << path << "</a> - "
        << toSizeString(doc.size) << " - "
        << h->mimetypeDescription(doc.mimetype) << "</div>";
    /*out << "<table>";
    map<string, string>::const_iterator j;
    for (j = doc.properties.begin(); j != doc.properties.end(); ++j) {
        out << "<tr><td>" << j->first << ":</td><td>" << j->second.c_str()
            << "</td></tr>";
    }
    out << "</table>";*/
    out << "</div>";
}
void
StrigiHtmlGui::Private::printSearchResults(ostream& out,
        const ClientInterface::Hits& hits, const string& q) const {
    Query query(q, 0, 0);
    vector<jstreams::IndexedDocument>::const_iterator i;
    for (i = hits.hits.begin(); i != hits.hits.end(); ++i) {
        printSearchResult(out, *i, query);
    }
}
