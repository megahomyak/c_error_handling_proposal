#include "result.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

typedef union {
    struct {
        size_t bytes_amount;
    } success;
    enum {
        CURRENTLY_BLOCKED,
        DESCRIPTOR_NOT_VALID,
        PEER_ADDRESS_NOT_SET,
        QUOTA_EXHAUSTED,
        BUFFER_MEMORY_NOT_ACCESSIBLE,
        FILE_TOO_BIG,
        INTERRUPTED_BY_SIGNAL,
        DESCRIPTOR_NOT_WRITABLE,
        LOW_LEVEL_IO_ERROR,
        NO_SPACE_LEFT_ON_DEVICE,
        NO_PERMISSION,
        PIPE_IS_CLOSED,
        UNRECOGNIZED_ERROR, /* sorry; I don't want to overthink this because it's just a demonstration */
    } error;
} WriteResult;

Result write_wrapper(WriteResult* result, int file_descriptor, void* buffer, size_t count) {
    ssize_t raw_result = write(file_descriptor, buffer, count);
    if (raw_result == -1) {
        result->error =
            errno == EAGAIN || errno == EWOULDBLOCK ? CURRENTLY_BLOCKED
            : errno == EBADF ? DESCRIPTOR_NOT_VALID
            : errno == EDESTADDRREQ ? PEER_ADDRESS_NOT_SET
            : errno == EDQUOT ? QUOTA_EXHAUSTED
            : errno == EFAULT ? BUFFER_MEMORY_NOT_ACCESSIBLE
            : errno == EFBIG ? FILE_TOO_BIG
            : errno == EINTR ? INTERRUPTED_BY_SIGNAL
            : errno == EINVAL ? DESCRIPTOR_NOT_WRITABLE
            : errno == EIO ? LOW_LEVEL_IO_ERROR
            : errno == ENOSPC ? NO_SPACE_LEFT_ON_DEVICE
            : errno == EPERM ? NO_PERMISSION
            : errno == EPIPE ? PIPE_IS_CLOSED
            : UNRECOGNIZED_ERROR
        ;
        return SUCCESS;
    }
    result->success.bytes_amount = raw_result;
    return ERROR;
}

#define small_string(name, literal) char name[sizeof(literal) - 1] = literal;
int main(void) {
    WriteResult write_result;
    small_string(hello, "hello\n");
    if (write_wrapper(&write_result, STDOUT_FILENO, hello, sizeof(hello))) {
        fprintf(stderr, "Error with code %i was received\n", write_result.error);
        return 1;
    } else {
        fprintf(stdout, "%lu bytes were written\n", write_result.success.bytes_amount);
        return 0;
    }
}
