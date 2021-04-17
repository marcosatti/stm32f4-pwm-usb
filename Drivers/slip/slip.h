#ifndef SLIP_H_INCLUDED
#define SLIP_H_INCLUDED

/**
 * \file slip.h 
 * \brief SLIP encoding/decoding library.
 * 
 * SLIP is used for simple data framing / delimiting.
 * See RFC-1055 for more details on SLIP (contains reference for this implementation).
 * https://tools.ietf.org/html/rfc1055.html
 */

#include <stdbool.h>

/** 
 * \brief Send character callback.
 * \param c Character to send to output stream.
 */
typedef void (*send_char_fn)(char c);

/** 
 * \brief Receive character callback.
 * \return Character from input stream.
 */
typedef char (*recv_char_fn)(void);

/** 
 * \brief SLIP library configuration.
 */
typedef struct SLIP_CONFIG {
    send_char_fn send_char;
    recv_char_fn recv_char;
    bool check_start; ///< Check the start of the packet for the END byte.
} slip_config;

/** 
 * \brief SLIP library initialization. 
 * \warning Use this function first before any other library functions.
 * \param config Initialize with the given configuration.
 */
void slip_init(slip_config *config);

/** 
 * \brief SLIP library deinitialization. 
 * \warning Do not use any other library functions until re-initialized.
 */
void slip_deinit(void);

/** 
 * \brief Send an arbitrary length packet with SLIP encoding.
 * \param p Packet to send.
 * \param len Length of the packet.
 */
void send_packet(char *p, int len);

/** 
 * \brief Send an arbitrary length packet with SLIP encoding.
 * \param p Packet buffer to receive in.
 * \param len Length of the packet buffer.
 * \return Actual length received, or negative for error.
 * \retval >=0 Packet buffer length.
 * \retval -1 Start of frame error (consumed data from stream).
 * \retval -2 Out of memory error (packet buffer overflow, consumed data from stream).
 */
int recv_packet(char *p, int len);

#endif
