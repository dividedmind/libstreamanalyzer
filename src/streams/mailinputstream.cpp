#include "jstreamsconfig.h"
#include "mailinputstream.h"
#include "subinputstream.h"
#include "stringterminatedsubstream.h"
#include "base64inputstream.h"
#include <cstring>
using namespace jstreams;
using namespace std;

/**
 * Validate a mail header. The header format is checked, but not the presence
 * of required fields. It is recommended to use a datasize of at least 512
 * bytes.
 **/
bool
MailInputStream::checkHeader(const char* data, int32_t datasize) {
    if (datasize < 512) return false;
    int linecount = 1;
    bool key = true;
    bool slashr = false;
    char prevc = 0;
    int32_t pos = 0;
    while (pos < datasize) {
        char c = data[pos++];
        if (slashr) {
            slashr = false;
            if (c == '\n') {
                continue;
            }
        }
        if (key) {
            if (c == ':' || (isblank(c) && isspace(prevc))) {
                // ':' signals the end of the key, a line starting with space
                // is a continuation of the previous line's value
                key = false;
            } else if ((c == '\n' || c == '\r') && linecount > 5
                    && (prevc == '\n' || prevc == '\r')) {
                // if at least 5 header lines were read and an empty line is
                // encountered, the mail header is valid
                return true;
            } else if (c != '-' && c != '.' && c != '_' && !isalnum(c)) {
                // an invalid character in the key
                return false;
            }
        } else {
            // check that the text is 7-bit
            //if (c <= 0) return false;
            if (c == '\n' || c == '\r') {
                // a new line starts, so a new key
                key = true;
                linecount++;
                // enable reading of \r\n line endings
                if (c == '\r') {
                    slashr = true;
                }
            }
        }
        prevc = c;
    }
    return true;
}
MailInputStream::MailInputStream(StreamBase<char>* input)
        : SubStreamProvider(input), entrystream(0), substream(0),
          bodysubstream(0) {
//    printf("%p\n", input);
    linenum = 0;
    skipHeader();
    if (bufstart == 0) {
        printf("no valid header\n");
        return;
    }

    // get the boundary
    boundary = getValue("boundary", contenttype);
}
MailInputStream::~MailInputStream() {
    if (entrystream) {
        delete entrystream;
    }
    if (substream) {
        delete substream;
    }
    if (bodysubstream) {
        delete bodysubstream;
    }
}
void
MailInputStream::readLine() {
    if (bufstart == 0) return;
    linenum++;
    linestart = lineend;
    bool backslashr = false;
    if (eol) {
        linestart++; // skip \r or \n
        backslashr = *lineend == '\r';
        //printf("%p %p %p %p\n", linestart, lineend, bufstart, bufend);
//        if (backslashr && linestart != bufend) printf("%i\n", *linestart);
        if (backslashr && linestart != bufend && *linestart == '\n') {
            // skip \n of \r\n
            linestart++;
        }
        lineend = linestart;
    }
    while (lineend != bufend && *lineend != '\n' && *lineend != '\r') {
        lineend++;
    }
    eol = true;
    if (lineend == bufend) {
        rewindToLineStart();
        fillBuffer();
        if (bufstart == 0) {
            // the input has been exhausted
            return;
        }
        if (backslashr && *linestart == '\n') {
            // we have to skip a \n because of a \r\n lineend across the read
            // boundary
            linestart++;
            if (linestart == bufend) {
                rewindToLineStart();
                fillBuffer();
                if (bufstart == 0) {
                    // the input has been exhausted
                    return;
                }
            }
        }
        lineend = linestart;
        while (lineend != bufend && *lineend != '\n' && *lineend != '\r') {
            lineend++;
        }
        if (lineend == bufend) {
            string str(linestart, 10);
            printf("line %i is too long '%s' %i %i\n", linenum, str.c_str(),
                lineend-linestart, maxlinesize);
            eol = false;
        }
    }
//    printf("%.*s\n", lineend-linestart, linestart);
}
string
MailInputStream::getValue(const char* n, const string& headerline) const {
    string name = n;
    name += "=";
    string value;
    // get the value
    const char* hl = headerline.c_str();
    const char* v = strcasestr(hl, name.c_str());
    if (v == 0) {
        // so far we just scan for a value attribute
        return value;
    }
    v += name.length();
    const char* vend = strchr(v, ';');
    if (vend == 0) {
        vend = hl + headerline.length();
    }
    if (*v == '"') {
        value = string(v+1, vend-v-2);
    } else {
        value = string(v, vend-v);
    }
    return value;
}
/**
 * Position the stream to the start of the active line. So in the next
 * call to this function, this line is read again.
 **/
void
MailInputStream::rewindToLineStart() {
    int64_t rp = bufstartpos + (linestart-bufstart);
    int64_t np = input->reset(bufstartpos + (linestart-bufstart));
    //printf("rewind %lli %lli\n", rp, np);
}
void
MailInputStream::fillBuffer() {
    bufstartpos = input->getPosition();
    int32_t nread = input->read(bufstart, maxlinesize, 0);
    if (nread > 0) {
//        printf("buf: '%.*s'\n", 10, bufstart);
        bufend = bufstart + nread;
        linestart = bufstart;
    } else {
        bufstart = 0;
    }
}
void
MailInputStream::skipHeader() {
    maxlinesize = 100;

    fillBuffer();
    lineend = bufstart;

    if (bufstart == 0) {
        // error: file too short
        return;
    }
    lastHeader = 0;
    eol = false;
    readLine();
    while (bufstart) {
        readLine();
        if (linestart == lineend) {
            break;
        }
        handleHeaderLine();
    }
    readLine();
    rewindToLineStart();
}
void
MailInputStream::scanBody() {
    while (bufstart) {
        readLine();
        if (boundary.length()+2 == size_t(lineend-linestart)
                && strncmp(boundary.c_str(), linestart+2, boundary.length())
                 == 0) {
            handleBodyLine();
            break;
        }
    }

//    const char* bend = strchr(bstart+1, '"');
 //   string boundary(bstart+1, bend-bstart-2);
//    printf("%s\n", contenttype.c_str());
//    printf("%s\n", boundary.c_str());
//    printf("%s %i\n", boundary.c_str(), n);
}
void
MailInputStream::handleHeaderLine() {
    static const char* subject = "Subject:";
    static const char* contenttype = "Content-Type:";
    static const char* contenttransferencoding = "Content-Transfer-Encoding:";
    static const char* contentdisposition = "Content-Disposition:";
    int32_t len = lineend - linestart;
    if (len < 2) return;
    if (lastHeader && isspace(*linestart)) {
        *lastHeader += string(linestart, len);
    } else if (len < 8) {
        lastHeader = 0;
        return;
    } else if (strncasecmp(linestart, subject, 8) == 0) {
        int32_t offset = 8;
        while (offset < len && isspace(linestart[offset])) offset++;
        this->subject = std::string(linestart+offset, len-offset);
        lastHeader = &this->subject;
    } else if (strncasecmp(linestart, contenttype, 13) == 0) {
        int32_t offset = 13;
        while (offset < len && isspace(linestart[offset])) offset++;
        this->contenttype = std::string(linestart+offset, len-offset);
        lastHeader = &this->contenttype;
    } else if (strncasecmp(linestart, contenttransferencoding, 26) == 0) {
        this->contenttransferencoding = std::string(linestart, len);
        lastHeader = &this->contenttransferencoding;
    } else if (strncasecmp(linestart, contentdisposition, 20) == 0) {
        this->contentdisposition = std::string(linestart, len);
        lastHeader = &this->contentdisposition;
    } else {
        lastHeader = 0;
    }
}
bool
MailInputStream::checkHeaderLine() const {
    bool validheader = bufstart && linestart != lineend;
    if (validheader) {
        const char* colpos = linestart;
        while (*colpos != ':' && ++colpos != lineend) {}
        validheader = colpos != lineend || isblank(*linestart);
    }
    return validheader;    
}
// return true if we are at the start of a base64 encoded block
void
MailInputStream::handleBodyLine() {
    clearHeaders();
    //printf("handleBodyLine %p %s\n", linestart, boundary.c_str());
    // start of new block
    // skip header
    bool validheader;
    do {
        readLine();
        validheader = checkHeaderLine();
        if (validheader) {
            handleHeaderLine();
        }
    } while (validheader);
    // set the stream to the start of the content
    readLine();
    if (bufstart == 0) return;
    rewindToLineStart();
    //printf("b %p %p %.*s\n", bufstart, linestart, lineend-linestart, linestart);

    // get the filename
    entryinfo.filename = getValue("filename", contentdisposition);
    if (entryinfo.filename.length() == 0) {
        entryinfo.filename = getValue("name", contenttype);
    }

    // create a stream that's limited to the content
    substream = new StringTerminatedSubStream(input, "--"+boundary);
    // set a reasonable buffer size
    substream->mark(10*boundary.length());
    //printf("%s\n", contenttransferencoding.c_str());
    if (strcasestr(contenttransferencoding.c_str(), "base64")) {
        //printf("base64 %p\n", substream);
        entrystream = new Base64InputStream(substream);
    }
}
StreamBase<char>*
MailInputStream::nextEntry() {
    if (status != Ok) return 0;
    // if the mail does not consist of multiple parts, we give a pointer to
    // the input stream
    if (boundary.length() == 0) {
        // signal eof because we only return eof once
        status = Eof;
        bodysubstream = new SubInputStream(input);
        return bodysubstream;
    }
    // read anything that's left over in the previous stream
    if (substream) {
        const char* dummy;
        while (substream->getStatus() == Ok) {
            substream->read(dummy, 1, 0);
        }
        if (substream->getStatus() == Error) {
            status = Error;
        } else {
            if (substream->getSize()<0) {
                printf("NONDEJU size should be determined %lli\n",
                    substream->getSize());
                status = Eof;
		// make sure valgrind is called
/*                substream = 0;
                substream->getSize();
                exit(0);*/
            }
            int64_t end = substream->getOffset()+substream->getSize()
                + boundary.length()+2;
            //printf("weird: %lli %lli\n",substream->getOffset(),substream->getSize());
            // skip to pass the boundary
            int64_t np = input->reset(end);
            if (np != end) {
                status = Error;
                printf("error: could not reset position\n");
            } else {
                int32_t nr = input->read(dummy, 1, 0);
                //printf("ohoh: %lli %lli '%.*s'\n",end,np, (nr>10)?10:nr,dummy);
                if (nr < 1 || *dummy == '-') {
                    // the end of the mail
                    status = Eof;
                }
                input->reset(end);
                input->read(dummy, 2, 2);
            }
        }

        delete substream;
        substream = 0;
        if (entrystream) {
            delete entrystream;
            entrystream = 0;
        }
        if (status != Ok) {
            return 0;
        }
        // force the stream to refresh the buffer
        fillBuffer();
        lineend = bufstart;
        handleBodyLine();
    } else {
        scanBody();
    }
    if (substream == 0) status = Eof;

    return entrystream ?entrystream : substream;
}
void
MailInputStream::clearHeaders() {
    contenttype.resize(0);
    contenttransferencoding.resize(0);
    contentdisposition.resize(0);
}

