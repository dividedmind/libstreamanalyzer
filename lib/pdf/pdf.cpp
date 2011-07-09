/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
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
#include <strigi/strigiconfig.h>
#include "pdfparser.h"
#include "fileinputstream.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#ifdef _WIN32
#include <direct.h>
#endif

using namespace Strigi;

class StreamHandler : public PdfParser::StreamHandler {
public:
    Strigi::StreamStatus handle(Strigi::StreamBase<char>* s) {
        static int count = 0;
        char name[32];
        const char *c;
        int32_t n = s->read(c, 1, 0);
        if (n <= 0) {
            return s->status();
        }
        sprintf(name, "out/%i", ++count);
        FILE* file = fopen(name, "wb");
        if (file == 0) {
            return Error;
        }
        do {
            fwrite(c, 1, n, file);
            n = s->read(c, 1, 0);
        } while (n > 0);
        fclose(file);
        return s->status();
    }
};

class TextHandler : public PdfParser::TextHandler {
public:
    Strigi::StreamStatus handle(const std::string& s) {
        printf("%s\n", s.c_str());
        return Ok;
    }
};

int
main(int argc, char** argv) {
#ifdef _WIN32
    mkdir("out");
#else
    mkdir("out", 0777);
#endif
    PdfParser parser;
    StreamHandler streamhandler;
    TextHandler texthandler;
    parser.setStreamHandler(&streamhandler);
    parser.setTextHandler(&texthandler);
    for (int i=1; i<argc; ++i) {
        // check if we can read the file
        FILE* f = fopen(argv[i], "rb");
        if (f == 0) continue;
        fclose(f);

        // parse the file
        FileInputStream file(argv[i]);
        try {
            StreamStatus r = parser.parse(&file);
        } catch (PdfParser::ParseError error) {
            std::cout << "error in " << argv[i] << ": " << error.what() << std::endl;
            const char *buf;
            file.read(buf, 0, 1024);
            std::cout << "rest: " << std::endl << buf;
        }
    }
    return 0;
}
