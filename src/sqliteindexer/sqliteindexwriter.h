#ifndef SQLITEINDEXWRITER_H
#define SQLITEINDEXWRITER_H

#include "indexwriter.h"
#include <map>

struct sqlite3;
struct sqlite3_stmt;
class SqliteIndexManager;

class SqliteIndexWriter : public jstreams::IndexWriter {
friend class SqliteIndexManager;
private:
    SqliteIndexManager* manager;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const std::string indexpath;
    std::map<int64_t, std::map<std::string, int> > content;

protected:
    void startIndexable(jstreams::Indexable*);
    void finishIndexable(const jstreams::Indexable*);
    void addStream(const jstreams::Indexable*, const std::string& fieldname,
        jstreams::StreamBase<wchar_t>* datastream);
    void addField(const jstreams::Indexable* idx, const std::string &fieldname,
        const std::string& value);
    void setField(const jstreams::Indexable*, const std::string &fieldname,
        int64_t value);
    SqliteIndexWriter(SqliteIndexManager*);
    ~SqliteIndexWriter();
};

#endif
