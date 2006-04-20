#ifndef BUFFEREDSTREAM_H
#define BUFFEREDSTREAM_H

#include "streambase.h"
#include "inputstreambuffer.h"

namespace jstreams {

template <class T>
class BufferedInputStream : public StreamBase<T> {
private:
    bool finishedWritingToBuffer;
    InputStreamBuffer<T> buffer;
protected:
    /**
     * This function must be implemented by the subclasses.
     * It should write a maximum of @p space characters at the buffer
     * position pointed to by @p start. If no more data is avaiable due to
     * error or end of file, -1 should be returned.
     **/
    virtual int32_t fillBuffer(T* start, int32_t space) = 0;
    void setBufferSize(int32_t buffersize) {
        buffer.setSize(buffersize);
    }
    void resetBuffer() {}
public:
    BufferedInputStream<T>();
    int32_t read(const T*& start);
    int32_t read(const T*& start, int32_t ntoread);
    StreamStatus mark(int32_t readlimit);
    StreamStatus reset();
};

template <class T>
BufferedInputStream<T>::BufferedInputStream<T>() {
    finishedWritingToBuffer = false;
}

template <class T>
int32_t
BufferedInputStream<T>::read(const T*& start) {
    if (StreamBase<T>::status == Error) return -1;
    if (StreamBase<T>::status == Eof) return 0;

    if (buffer.avail < 1) {
        int32_t nwritten;
        do {
            T* first = buffer.readPos + buffer.avail;
            int32_t space;
            nwritten = fillBuffer(first, space);
            if (nwritten > 0) {
                buffer.avail += nwritten;
            }
        } while (StreamBase<T>::status == Ok
                && nwritten >= 0 && buffer.avail < 1);
        if (nwritten < 0) {
            finishedWritingToBuffer = true;
        }
        if (StreamBase<T>::status == Error) return -1;
        if (StreamBase<T>::status == Eof) return 0;
    }

    return buffer.read(start);
}
template <class T>
int32_t
BufferedInputStream<T>::read(const T*& start, int32_t ntoread) {
    if (StreamBase<T>::status == Error) return -1;
    if (StreamBase<T>::status == Eof) return 0;

    int32_t missing = ntoread - buffer.avail;
    if (missing > 0) {
        // check if there is enough space in the buffer
        int32_t space = buffer.size - (buffer.readPos - buffer.start)
            - buffer.avail;
        if (space < missing) { // buffer is too small
            // may we move the data in the buffer?
            bool maymove = (buffer.markPos == 0
                || buffer.readPos - buffer.markPos > buffer.markLimit);
        }
        int32_t nwritten;
        do {
            T* first;
            int32_t space;
            nwritten = fillBuffer(first, space);
            if (nwritten > 0) {
                buffer.avail += nwritten;
            }
        } while (StreamBase<T>::status == Ok
                && nwritten >= 0 && buffer.avail < ntoread);
        if (nwritten < 0) {
            finishedWritingToBuffer = true;
        }
        if (StreamBase<T>::status == Error) return -1;
        if (StreamBase<T>::status == Eof) return 0;
    }

    printf("read1 %p %p\n", this, buffer.markPos);
    int32_t nread = buffer.read(start, ntoread);
    printf("read2 %p %p %i %i\n", this, buffer.markPos, ntoread, nread);
    return nread;
}
template <class T>
StreamStatus
BufferedInputStream<T>::mark(int32_t readlimit) {
    printf("mark %p %i\n", this, readlimit);
    buffer.mark(readlimit);
    return Ok;
}
template <class T>
StreamStatus
BufferedInputStream<T>::reset() {
    printf("reset %p %p\n", this, buffer.markPos);
    if (buffer.markPos) {
        buffer.reset();
        return Ok;
    } else {
        StreamBase<T>::error = "No valid mark for reset.";
        return Error;
    }
}
}

#endif
