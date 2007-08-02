/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Jos van den Oever <jos@vandenoever.info>
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
#include "combinedindexmanager.h"
#include "indexreader.h"
#include "indexwriter.h"
#include "indexeddocument.h"
#include "analyzerconfiguration.h"
#include "diranalyzer.h"
#include "strigiconfig.h"
#include "query.h"
#include "queryparser.h"
#include "XesamQLParser.h"
#include "StrigiQueryBuilder.h"
#include <algorithm>
#include <string>
#include <set>
#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <time.h>
#include "stgdirent.h" //dirent replacement (includes native if available)

using namespace std;
using namespace Strigi;

map<char, string> options;
vector<string> arguments;

AnalyzerConfiguration::FieldType
operator|(AnalyzerConfiguration::FieldType a, AnalyzerConfiguration::FieldType b){
    return static_cast<AnalyzerConfiguration::FieldType>((int)a|(int)b);
}

class CustomAnalyzerConfiguration: public AnalyzerConfiguration {
public:

//    None       = 0x0000 /**< No hint. */,
//    Binary     = 0x0001 /**< The field should be stored as binary data. */,
//    Compressed = 0x0002 /**< If the field is stored, the data
//                             should be compressed. */,
//    Indexed    = 0x0004 /**< The field should be indexed. */,
//    Stored     = 0x0020 /**< The field should be stored. */,
//    Tokenized  = 0x0040 /**< If the field contains text, it
//                             should be tokenized. */
    AnalyzerConfiguration::FieldType indexType(const RegisteredField* field) const {
	if (string("chemistry.inchi").compare(field->key()) == 0) {
	    return Stored|Indexed;
	} else if (string("chemistry.molecular_formula").compare(field->key()) == 0) {
	    return Stored|Indexed;
/*	} else if (string("chemistry.").compare(field->key()) == 0) {
	    return Binary|Stored|Indexed;
	} else if (string("chemistry.").compare(field->key()) == 0) {
	    return Binary|Stored|Indexed;
	} else if (string("chemistry.").compare(field->key()) == 0) {
	    return Binary|Stored|Indexed; */
	} else {
	    return Tokenized|Stored|Indexed;
	}
    }
};

void
parseArguments(int argc, char** argv) {
    // parse arguments
    int i = 1;
    while (++i < argc) {
        const char* arg = argv[i];
        size_t len = strlen(arg);
        if (len == 0) continue;
        if (*arg == '-' && len > 1) {
            char option = arg[1];
            if (len > 2) {
                arg += 2;
            } else if (i+1 < argc) {
                arg = argv[++i]; 
            } else {
                arg = "";
            }
            options[option].assign(arg);
        } else {
            arguments.push_back(arg);
        }
    }
}
/*
 * Help function for printing to stderr: fprintf(stderr, 
 */
int
pe(const char *format, ...) {
    va_list arg;
    int done;
    va_start(arg, format);
    done = vfprintf(stderr, format, arg);
    va_end(arg);
    return done;
}
/**
 * This is the main for implementing a command line program that can create
 * and query indexes.
 **/
int
usage(int argc, char** argv) {
    pe("%s:\n", argv[0]);
    pe("    Program for creating and querying indices.\n");
    pe("    This program is not meant for talking to the strigi daemon.\n\n");
    pe("usage:\n");
    pe("  %s create [-j num] -t backend -d indexdir [-i include] [-x exclude] files/dirs\n");
    pe("  %s deindex -t backend -d indexdir files/dirs\n");
    pe("  %s get -t backend -d indexdir files\n");
    pe("  %s listFiles -t backend -d indexdir\n");
    pe("  %s listFields -t backend -d indexdir\n");
    //TODO: find a better definition for query?
    pe("  %s query -t backend -d indexdir queries\n");
    pe("  %s xesamquery -t backend -d indexdir [-u xesam_user_language_file] ");
    pe("[-q xesam_query_language_file]\n");
    pe("  %s update [-j num] -t backend -d indexdir [-i include] [-x exclude] files/dirs\n");
    return 1; 
}
void
checkIndexdirIsEmpty(const char* dir) {
    DIR* d = opendir(dir);
    if (!d) return;
    struct dirent* de = readdir(d);
    while (de) {
        if (strcmp(de->d_name, "..") && strcmp(de->d_name, ".")) {
            fprintf(stderr, "Directory %s is not empty.\n", dir);
            exit(1);
        }
        de = readdir(d);
    }
    closedir(d);
}
void
printBackends(const string& msg, const vector<string> backends) {
    pe(msg.c_str());
    pe(" Choose one from ");
    for (uint j=0; j<backends.size()-1; ++j) {
        pe("'%s', ", backends[j].c_str());
    }
    pe("'%s'.\n", backends[backends.size()-1].c_str());
}
void
printIndexedDocument(IndexedDocument indexedDoc) {
    printf ("\t- mimetype: %s\n", indexedDoc.mimetype.c_str());
    printf ("\t- sha1: %s\n", indexedDoc.sha1.c_str());
    cout << "\t- size: " << indexedDoc.size << endl;
    const time_t mtime = (const time_t) indexedDoc.mtime;
    printf ("\t- mtime: %s", ctime (&mtime));
    printf ("\t- fragment: %s", indexedDoc.fragment.c_str());
    set<string> processedProperties;
    for (multimap<string,string>::iterator iter = indexedDoc.properties.begin();
         iter != indexedDoc.properties.end();
         iter++) {
        // iter over all document properties
        
        set<string>::iterator match = processedProperties.find(iter->first);
        if (match != processedProperties.end())
            continue;
        
        processedProperties.insert (iter->first);
        multimap<string,string>::iterator it;
        bool first = true;
        for (it = indexedDoc.properties.lower_bound(iter->first);
             it != indexedDoc.properties.upper_bound(iter->first); it++) {
            // shows all properties with the same key together
            if (first) {
                printf ("\t- %s:\t%s\n", it->first.c_str(), it->second.c_str());
                first = false;
            }
            else
                printf ("\t\t%s\n", it->second.c_str());
        }
    }
}

/*!
    Convenience class used with find_if statement. It returns all the indexed
    files whose path starts with ref
 */
class FindIndexedFiles {
private:
    string m_ref;
public:
    FindIndexedFiles(string ref) {
        m_ref = ref;
    }
    bool operator()(pair<string, time_t> indexedFile) {
        string::size_type pos = indexedFile.first.find (m_ref);
        if (pos == 0) {
            return true;
        }
        return false;
    }
};

IndexManager*
getIndexManager(string& backend, const string& indexdir) {
    // check arguments: backend
    const vector<string>& backends = CombinedIndexManager::backEnds();
    // if there is only one backend, the backend does not have to be specified
    if (backend.size() == 0) {
        if (backends.size() == 1) {
            backend = backends[0];
        } else {
            printBackends("Specify a backend.", backends);
            return 0;
        }
    }
    vector<string>::const_iterator b
        = find(backends.begin(), backends.end(), backend);
    if (b == backends.end()) {
        printBackends("Invalid index type.", backends);
        return 0;
    }
    return CombinedIndexManager::factories()[backend](indexdir.c_str());
}
int
create(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];
    string excluded_filter = options['x'];
    string included_filter = options['i'];
    int nthreads = atoi(options['j'].c_str());
    if (nthreads < 1) nthreads = 2;

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide a dir to write the index to with -d.\n");
        return usage(argc, argv);
    }
    // check that the dir does not yet exist
    checkIndexdirIsEmpty(indexdir.c_str());

    // check arguments: dirs
    if (arguments.size() == 0) {
        pe("'%s' '%s'\n", backend.c_str(), indexdir.c_str());
        pe("Provide directories to index.\n");
        return usage(argc, argv);
    }

    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }

    vector<pair<bool,string> >filters;
    filters.push_back(make_pair<bool,string>(true,included_filter));
    filters.push_back(make_pair<bool,string>(false,excluded_filter));
    
    //CustomAnalyzerConfiguration config;
    AnalyzerConfiguration config;
    config.setFilters(filters);

    DirAnalyzer* analyzer = new DirAnalyzer(*manager, config);
    vector<string>::const_iterator j;
    for (j = arguments.begin(); j != arguments.end(); ++j) {
        analyzer->analyzeDir(j->c_str(), nthreads);
    }
    delete analyzer;
    delete manager;

    return 0;
}
int
update(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];
    string excluded_filter = options['x'];
    string included_filter = options['i'];
    int nthreads = atoi(options['j'].c_str());
    if (nthreads < 1) nthreads = 2;

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide a dir to write the index to with -d.\n");
        return usage(argc, argv);
    }

    // check arguments: dirs
    if (arguments.size() == 0) {
        pe("'%s' '%s'\n", backend.c_str(), indexdir.c_str());
        pe("Provide directories to index.\n");
        return usage(argc, argv);
    }

    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }

    vector<pair<bool,string> >filters;
    filters.push_back(make_pair<bool,string>(true, included_filter));
    filters.push_back(make_pair<bool,string>(false, excluded_filter));
    
    AnalyzerConfiguration config;
    config.setFilters(filters);

    DirAnalyzer* analyzer = new DirAnalyzer(*manager, config);
    analyzer->updateDirs(arguments, nthreads);
    delete analyzer;
    delete manager;

    return 0;
}
int
listFiles(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide the directory with the index.\n");
        return usage(argc, argv);
    }

    // create an index manager
    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }
    IndexReader* reader = manager->indexReader();
    map<string, time_t> files = reader->files(-1);
    map<string, time_t>::const_iterator i;
    for (i=files.begin(); i!=files.end(); ++i) {
        printf("%s\n", i->first.c_str());
    }
    delete manager;
    return 0;
}
int
get(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide the directory with the index.\n");
        return usage(argc, argv);
    }

    // check arguments: dirs
    if (arguments.size() == 0) {
        pe("'%s' '%s'\n", backend.c_str(), indexdir.c_str());
        pe("Provide one or more files to search.\n");
        return usage(argc, argv);
    }
    
    // create an index manager
    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }
    IndexReader* reader = manager->indexReader();
    QueryParser parser;
    
    for (vector<string>::iterator iter = arguments.begin();
         iter != arguments.end(); iter++) {
        Query query = parser.buildQuery( "system.location:'"+ *iter + '\'');
        vector<IndexedDocument> matches = reader->query(query, 0, 10);
        if (matches.size() == 0)
            printf ("%s: is not indexed\n", iter->c_str());
        else
        {
            printf ("Informations associated to %s:\n", iter->c_str());
            for (vector<IndexedDocument>::iterator it = matches.begin();
                 it != matches.end(); it++)
            {
                printIndexedDocument(*it);
            }
        }
    }
    
    delete manager;
    return 0;
}
int
query(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide the directory with the index.\n");
        return usage(argc, argv);
    }

    // check arguments: dirs
    if (arguments.size() == 0) {
        pe("'%s' '%s'\n", backend.c_str(), indexdir.c_str());
        pe("Provide one or more files to search.\n");
        return usage(argc, argv);
    }
    
    // create an index manager
    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }
    IndexReader* reader = manager->indexReader();
    QueryParser parser;
    
    for (vector<string>::iterator iter = arguments.begin();
            iter != arguments.end(); iter++) {
        unsigned int results = 0;
        Query query = parser.buildQuery(*iter);
        const uint batchsize = 10;
        vector<IndexedDocument> matches = reader->query(query, 0, batchsize);

        if (matches.size() != 0) {
            printf ("Results for search \"%s\"\n", (*iter).c_str());
        } else {
            printf ("No results for search \"%s\"\n", (*iter).c_str());
        }

        while (matches.size() > 0) {
            for (vector<IndexedDocument>::iterator it = matches.begin();
                 it != matches.end(); it++) {
                printf ("\"%s\" matched\n", it->uri.c_str());
                printIndexedDocument(*it);
            }

            results += matches.size();
            
            if (matches.size() == batchsize) {
                // maybe there're other results
                matches = reader->query(query, results + 1, 10);
            } else {
                matches.clear(); // force while() exit
            }
        }

        if (results != 0) {
            printf ("Query \"%s\" returned %i results\n", (*iter).c_str(),
                                                           results);
        }
    }
    
    delete manager;
    return 0;
}
int
xesamquery(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];
    string ulfile = options['u'];
    string qlfile = options['q'];

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide the directory with the index.\n");
        return usage(argc, argv);
    }

    if ((ulfile.length() == 0) && (qlfile.length() == 0)) {
        pe ("Provide at last one file containing the xesam query formulated ");
        pe ("with query language or user language.\n");
        return usage(argc, argv);
    }
    
    if (arguments.size() != 0) {
        pe("You don't have to provide other arguments.\n");
        return usage(argc, argv);
        return 1;
    }

    Dijon::StrigiQueryBuilder strigiQueryBuilder;

    if (qlfile.length() != 0) {
        printf ("processing query from file %s\n", qlfile.c_str());

        Dijon::XesamQLParser xesamQlParser;

        if (xesamQlParser.parse_file ( qlfile, strigiQueryBuilder))
            printf ("Xesam query parsed successfully\n");
        else
        {
            pe ("error parsing query\n");
            return 1;
        }
    }
    else if (ulfile.length() != 0) {
        pe ("xesam user language queries are not yet supported\n");
//         Dijon::XesamULParser xesamUlParser;
// 
//         if (xesamUlParser.parse_file ( ulfile, strigiQueryBuilder))
//             printf ("query parsed successfully\n");
//         else
//         {
//             pe ("error parsing query\n");
//             return 1;
//         }
    }
    
    // create an index manager
    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }
    IndexReader* reader = manager->indexReader();

    unsigned int results = 0;
    const uint batchsize = 10;
    Query query = strigiQueryBuilder.get_query();
    vector<IndexedDocument> matches = reader->query(query, 0, batchsize);

    if (matches.size() != 0) {
        printf ("Search results:\n");
    } else {
        printf ("No results for search\n");
    }

    while (matches.size() > 0) {
        for (vector<IndexedDocument>::iterator it = matches.begin();
            it != matches.end(); it++) {
                printf ("\"%s\" matched\n", it->uri.c_str());
                printIndexedDocument(*it);
            }

        results += matches.size();

        if (matches.size() == batchsize) {
            // maybe there're other results
            matches = reader->query(query, results + 1, 10);
        } else {
            matches.clear(); // force while() exit
        }
    }

    if (results != 0)
        printf ("Query returned %i results\n", results);

    delete manager;
    return 0;
}
int
deindex(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide the directory with the index.\n");
        return usage(argc, argv);
    }

    // check arguments: dirs
    if (arguments.size() == 0) {
        pe("'%s' '%s'\n", backend.c_str(), indexdir.c_str());
        pe("Provide directories/files to deindex.\n");
        return usage(argc, argv);
    }
    
    AnalyzerConfiguration config;
    
    // create an index manager
    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }
    
    // retrieve all indexed files at level 0 (archives contents aren't returned)
    IndexReader* indexReader = manager->indexReader();
    map <string, time_t> indexedFiles = indexReader->files(0);
    vector<string> toDelete;
    
    for (vector<string>::iterator iter = arguments.begin();
         iter != arguments.end(); iter++)
    {
        // find all indexed files whose path starts with *iter
        FindIndexedFiles match (*iter);
        map<string, time_t>::iterator itBeg, itEnd, itMatch;
        itBeg = indexedFiles.begin();
        itEnd = indexedFiles.end();
        
        itMatch = find_if (itBeg, itEnd, match);
        while (itMatch != itEnd)
        {
            toDelete.push_back (itMatch->first);
            itBeg = ++itMatch;
            itMatch = find_if (itBeg, itEnd, match);
        }
    }
    
    if (toDelete.empty())
        printf ("no file will be deindexed\n");
    else
    {
        for (vector<string>::iterator iter = toDelete.begin();
             iter != toDelete.end(); iter++)
        {
            printf ("%s will be deindex\n", iter->c_str());
        }
        IndexWriter* writer = manager->indexWriter();
        writer->deleteEntries(toDelete);
        writer->commit();
        writer->optimize();
    }
    
    delete manager;
    return 0;
}
int
listFields(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];

    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide the directory with the index.\n");
        return usage(argc, argv);
    }

    // create an index manager
    IndexManager* manager = getIndexManager(backend, indexdir);
    if (manager == 0) {
        return usage(argc, argv);
    }
    IndexReader* reader = manager->indexReader();
    vector<string> fields = reader->fieldNames();
    vector<string>::const_iterator i;
    for (i=fields.begin(); i!=fields.end(); ++i) {
        printf("%s\n", i->c_str());
    }
    delete manager;
    return 0;
}
int
main(int argc, char** argv) {
    if (argc < 2) { 
        return usage(argc, argv);
    }
    const char* cmd = argv[1];
    if (!strcmp(cmd,"create")) {
        return create(argc, argv);
    } else if (!strcmp(cmd,"update")) {
        return update(argc, argv);
    } else if (!strcmp(cmd,"listFiles")) {
        return listFiles(argc, argv);
    } else if (!strcmp(cmd,"listFields")) {
        return listFields(argc, argv);
    } else if (!strcmp(cmd,"get")) {
        return get(argc, argv);
    } else if (!strcmp(cmd,"deindex")) {
        return deindex(argc, argv);
    } else if (!strcmp(cmd,"query")) {
        return query(argc, argv);
    } else if (!strcmp(cmd,"xesamquery")) {
        return xesamquery(argc, argv);
    } else {
        return usage(argc, argv);
    }
    return 0;
}
