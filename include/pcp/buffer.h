/*
    This file is part of Pretty Curved Privacy (pcp1).

    Copyright (C) 2013-2014 T.v.Dein.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    You can contact me by mail: <tom AT vondein DOT org>.
*/

#ifndef HAVE_PCP_BUFFER_H
#define HAVE_PCP_BUFFER_H

#include <stdio.h>
#include <stdarg.h>
#include "mem.h"
#include "util.h"
#include "defines.h"

/**
 * \defgroup Buffer Buffer, a flexible buffer management class.
 * @{

  Flexible buffer management, idea from openssh/buffer.c.

  This class allows us to dissect buffers into parts at will
  whithout the hassle of boundary checking in each and every
  line. Therefore it is more secure, since this system wraps
  all this stuff from us, so in case we're attemt to overflow
  a buffer or the like, the buffer functions will catch this,
  warn us and die.

 */


/** \struct _pcp_buffer
    A flexible buffer object wich automatically resizes, if neccessary.
*/
struct _pcp_buffer {
  char *name;        /**< just for convenience in error messages and the like, so we know which buffer cause trouble */
  uint8_t allocated; /**< marks the buffer as allocated */
  size_t blocksize;  /**< the blocksize to use when resizing, also used for initial malloc() */
  size_t size;       /**< stores the current allocated size of the object */
  size_t offset;     /**< current read position */
  size_t end;        /**< current write position, data end. maybe less than size. */
  uint8_t isstring;  /**< treat as char array/string */
  void *buf;         /**< the actual storage buffer */
};

/** The name used everywhere */
typedef struct _pcp_buffer Buffer;

/** Create a new buffer.

    Create a new buffer, initially alloc'd to blocksize and zero-filled.

    \param[in] blocksize Initial blocksize. The smaller the more often
                     the buffer will be resized. Choose with care.

    \param[in] name A name for the Buffer. Just used for debugging purposes or in error messages.

    \return Returns a new Buffer object.
*/
Buffer *buffer_new(size_t blocksize, char *name);

/** Create a new buffer.

    Create a new buffer, initially alloc'd to a blocksize of 32 bytes and zero-filled.
    The buffer will be a string buffer. See buffer_get_str().

    \param[in] name A name for the Buffer. Just used for debugging purposes or in error messages.

    \return Returns a new Buffer object.
*/
Buffer *buffer_new_str(char *name);

/* initialize buffer vars */
void buffer_init(Buffer *b, size_t blocksize, char *name);

/** Clears and frees the Buffer.

    This clears the buffer by filling it with zeroes and frees any
    allocated memory, including the Buffer object itself. Use this
    function instead of directly calling free(Buffer).

    \param[in] b The Buffer object.
*/
void buffer_free(Buffer *b);

/** Clears the Buffer.

    This clears the buffer by filling it with zeroes and resetting
    all counters. Memory will not be free'd. Called from buffer_free()
    before free'ing memory.

    \param[in] b The Buffer object.
*/
void buffer_clear(Buffer *b);

/** Put read offset back to start.

    This function sets the read offset counter back to 0 (start
    of the buffer).

    \param[in] b The Buffer object.
 */
void buffer_rewind(Buffer *b);

/** Add data to the buffer.

    Adds data of the size len to the buffer and resizes the
    buffer, if neccessary. The write position ('end' field)
    will be updated accordingly.

    Data will be copied, you can free() the given pointer after copying..

    \param[in] b The Buffer object.

    \param[out] data Arbitrary data to add to the Buffer.

    \param[in] len The size of the data to add in Bytes.
 */
void buffer_add(Buffer *b, const void *data, size_t len);

/** Add data to the buffer.

    Adds data from the given Buffer src to the buffer and resizes the
    buffer, if neccessary. The write position ('end' field)
    will be updated accordingly.

    Data will be copied, you can buffer_free() the given src Buffer after the copying.

    \param[out] dst The destination Buffer object to copy data into.

    \param[in] src The source Buffer object to copy data from.
 */
void buffer_add_buf(Buffer *dst, Buffer *src);

/** Add a formated string to the buffer.

    Use printf() like syntax to add a formatted string
    to the buffer. Refer to the documentation of printf() for
    details.

    Data will be copied, you can free() the given format string and params after copying.

    Example:
    @code
    Buffer *x = buffer_new_str("test");
    buffer_add_str(x, "There are %d elements left in %s\n", 4, "list");
    @endcode

    \param[in] b The Buffer object.

    \param[in] fmt The printf() compatible format description.

    \param[in] ... A variable number of arguments for the format string.
 */
void buffer_add_str(Buffer *b, const char * fmt, ...);

/** Add data as hex string to the buffer.

    Adds data of the size len to the buffer and resizes the
    buffer, if neccessary. The write position ('end' field)
    will be updated accordingly. Each byte will be put in its
    HEX form into the buffer (%02x).

    Data will be copied, you can free() the given pointer after copying..

    \param[in] b The Buffer object.

    \param[in] data Arbitrary data to add as hex into the Buffer.

    \param[in] len The size of the data to add in Bytes.
 */
void buffer_add_hex(Buffer *b, void *data, size_t len);

/* resize the buffer if necessary, used internally only */
void buffer_resize(Buffer *b, size_t len);

/** Tell if there are no more bytes to read.

    This functions tells if the EOF of the buffer is reached
    during read operations (no more data to read left).

    \param[in] b The Buffer object.

    \return Returns 1 of EOF has been reached or 0 if there are more data left to read.
*/
int buffer_done(Buffer *b);

/** Read some chunk of data from the Buffer.

    Read some chunk of data from the Buffer, starting from current read
    offset til len.

    Example: suppose you've got a buffer with the following content:

    @code
    AAAABBBBCCCC
    @endcode

    Then the following code would:

    @code
    unsigned char g[4];
    buffer_get_chunk(b, g, 4);  // => g now contains 'AAAA' 
    buffer_get_chunk(b, g, 4);  // => g now contains 'BBBB' 
    buffer_get_chunk(b, g, 4);  // => g now contains 'CCCC' 
    @endcode

    In order to catch buffer overflow, check the return value, which will
    be 0 in case of errors. See also: fatals_ifany(), buffer_done() and buffer_left().

    \param[in] b The Buffer object to read from.

    \param[out] buf The destination pointer where the data will be copied to. This pointer
must be allocated by the caller properly and it must have at least a size of len.

    \param[in] len The number of bytes to read from the Buffer.

*/
size_t buffer_get_chunk(Buffer *b, void *buf, size_t len);

/** Read the whole Buffer content.

    This function returns the whole buffer contents as a pointer
    to the internal data member (Buffer->buf). The returned pointer
    is allocated and filled with data up to buffer_size(Buffer),
    however, the allocated memory might be more than size, in fact
    it will be a multitude of Buffer-blocksize.

    Don't free() the pointer directly, use buffer_free() always.

    \param[in] b The Buffer object to read from.
     
    \return Pointer to the buffer data storage. 
*/
unsigned char *buffer_get(Buffer *b);

/** Read the whole Buffer content as string.

    Access the Buffer content as string (char *).

    The returned pointer
    is allocated and filled with data up to buffer_size(Buffer),
    however, the allocated memory might be more than size, in fact
    it will be a multitude of Buffer-blocksize.

    The byte after buffer_size(Buffer) will be a \0.

    Don't free() the pointer directly, use buffer_free() always.

    Sample usage:

    @code
    [..]
    fprintf(stdout, "Our buffer content: %s\n", buffer_get_str(b));
    @endcode

    \param[in] b The Buffer object to read from.
     
    \return Pointer to the buffer data storage. 
*/
char *buffer_get_str(Buffer *b);

/** Read the remaining data after current read offset.

    Fetch whatever is left in the buffer. This works like
    buffer_get() but instead doesn't return everything,
    but only the part of the buffer, which follows after
    the current read offset.

    The returned pointer will be allocated by buffer_get_remainder()
    with a size of buffer_left(). It's up to the caller to free()
    the returned pointer later on.

    Example: suppose you've got a buffer with the following content:

    @code
    AAAABBBBCCCC
    @endcode
    
    Then:

    @code
    [..]
    unsigned char g[4];
    unsigned char *r = NULL;
    buffer_get_chunk(b, g, 4);    // => g now contains 'AAAA'
    size_t rs = buffer_left(b);   // => rs = 8
    r = buffer_get_remainder(b);  // => r now contains 'BBBBCCCC' and has a size of 8
    memset(r, 0, rs);             // zerofill r
    free(r);                      // done with it
    @endcode

    \param[in] b The Buffer object to read from.
     
    \return Pointer to the remaining chunk of data (copy).
*/
unsigned char *buffer_get_remainder(Buffer *b);

/** Read some data inside the Buffer.

    Same as buffer_get() but fetch some data chunk from somewhere
    in the middle of the buffer.

    The returned pointer has to be allocated by the caller to
    at least a size of len bytes.

    The read offset will be left untouched by this function.

    Example: suppose you've got a buffer with the following content:

    @code
    AAAABBBBCCCC
    @endcode

    Then:
    @code
    [..]
    unsigned char g[4];
    buffer_extract(b, g, 4, 4);  // => g now contains 'BBBB'
    @endcode

    \param[in] b The Buffer object to read from.

    \param[out] buf The buffer to copy data to.

    \param[in] offset Where to start copying.

    \param[in] len How mush data to copy.

    \return Returns the size of bytes read. Returns 0 in case of
    an overflow, which can be catched with fatals_ifany().

*/
size_t buffer_extract(Buffer *b, void *buf, size_t offset, size_t len);

/** Dump the Buffer contents to stderr in hex form.

    \param[in] b The Buffer object to dump.
*/
void buffer_dump(const Buffer *b);

/** Print Buffer counters to stderr.

    \param[in] b The Buffer object to print infos about.
 */
void buffer_info(const Buffer *b);

/** Tell how much data there is in the buffer available.

    This function returns the number of bytes stored in the
    buffer so far. Please note, that the actual allocation might
    be bigger, because we always allocate memory blockwise.

    \param[in] b The Buffer object to get the size from.

    \return The number of bytes stored in the Buffer.
 */
size_t buffer_size(const Buffer *b);

/** Tell how much data is left to read in the Buffer.

    Use this function to check if it's ok to read more
    bytes from to buffer to avoid buffer overflows.

    Example: suppose you've got a buffer with the following content:

    @code
    AAAABBBBCCCC
    @endcode

    Then:
    @code
    [..]
    unsigned char g[4];
    unsigned char x[16];
    buffer_get_chunk(b, g, 4);  // => g now contains 'BBBB'
    if(buffer_left(b) >= 16)    // => will return 8 and therefore fail
      buffer_get_chunk(b, x, 16);
    else
      printf("not enough data");  // => will be printed
    @endcode

    \param[in] b The Buffer object to get the size from.

    \return The number of bytes left to read from the Buffer.

 */
size_t buffer_left(const Buffer *b);

/** Read 1 byte (8 bit) number from a Buffer.

    \param[in] b The Buffer object to read from.

    \return a uint8_t.
 */
uint8_t buffer_get8(Buffer *b);

/** Read 2 bytes (16 bit) number from a Buffer.

    \param[in] b The Buffer object to read from.

    \return a uint16_t.
 */
uint16_t buffer_get16(Buffer *b);

/** Read 4 byte (32 bit) number from a Buffer.

    \param[in] b The Buffer object to read from.

    \return a uint32_t.
 */
uint32_t buffer_get32(Buffer *b);

/** Read 8 byte (64 bit) from a Buffer.

    \param[in] b The Buffer object to read from.

    \return a uint64_t.
 */
uint64_t buffer_get64(Buffer *b);

/** Read 2 bytes (16 bit) number from a Buffer, converted to host endian.

    \param[in] b The Buffer object to read from.

    \return a uint16_t.
 */
uint16_t buffer_get16na(Buffer *b);

/** Read 4 byte (32 bit) number from a Buffer, converted to host endian.

    \param[in] b The Buffer object to read from.

    \return a uint32_t.
 */
uint32_t buffer_get32na(Buffer *b);

/** Read 8 byte (64 bit) from a Buffer, converted to host endian.

    \param[in] b The Buffer object to read from.

    \return a uint64_t.
 */
uint64_t buffer_get64na(Buffer *b);

/** Read the last 1 byte (8 bit) number from a Buffer.

    Doesn't increment offset.

    \param[in] b The Buffer object to read from.

    \return a uint8_t.
 */
uint8_t  buffer_last8(Buffer *b);

/** Read the last 2 byte (16 bit) number from a Buffer.

    Doesn't increment offset.

    \param[in] b The Buffer object to read from.

    \return a uint16_t.
 */
uint16_t buffer_last16(Buffer *b);

/** Read the last 4 byte (32 bit) number from a Buffer.

    Doesn't increment offset.

    \param[in] b The Buffer object to read from.

    \return a uint32_t.
 */
uint32_t buffer_last32(Buffer *b);

/** Read the last 8 byte (64 bit) number from a Buffer.

    Doesn't increment offset.

    \param[in] b The Buffer object to read from.

    \return a uint64_t.
 */
uint64_t buffer_last64(Buffer *b);

/** Read data from a file directly into a Buffer.

    This function reads in len bytes from the FILE stream
    'in' into the Buffer. The file must already be opened
    by the caller.

    \param[in,out] b The Buffer object to read from.

    \param[in] in The FILE stream to read from.

    \param[in] len The number of bytes to read.

    \return Returns the number of bytes read or 0 in case of an error or EOF.
            Use feof() and ferror() to check this afterwards, call fatals_ifany()
            in case of errors.
 */
size_t buffer_fd_read(Buffer *b, FILE *in, size_t len);

/** Write a 1 byte (8 bit) number in binary form into the buffer.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint8_t to write to the buffer.
*/  
void buffer_add8(Buffer *b, uint8_t v);

/** Write a 2 byte (16 bit) number in binary form into the buffer.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint16_t to write to the buffer.
*/ 
void buffer_add16(Buffer *b, uint16_t v);

/** Write a 4 byte (32 bit) number in binary form into the buffer.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint32_t to write to the buffer.
*/ 
void buffer_add32(Buffer *b, uint32_t v);

/** Write a 8 byte (64 bit) number in binary form into the buffer.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint64_t to write to the buffer.
*/ 
void buffer_add64(Buffer *b, uint64_t v);

/** Write a 2 byte (16 bit) number in binary form into the buffer, converted to big endian.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint16_t to write to the buffer.
*/ 
void buffer_add16be(Buffer *b, uint16_t v);

/** Write a 4 byte (32 bit) number in binary form into the buffer, converted to big endian.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint32_t to write to the buffer.
*/ 
void buffer_add32be(Buffer *b, uint32_t v);

/** Write a 8 byte (64 bit) number in binary form into the buffer, converted to big endian.

    \param[out] b The Buffer object to write to.

    \param[in] v The uint64_t to write to the buffer.
*/ 
void buffer_add64be(Buffer *b, uint64_t v);


#endif // HAVE_PCP_BUFFER_H

/**@}*/
