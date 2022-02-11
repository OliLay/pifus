#ifndef PIFUS_BYTE_BUFFER
#define PIFUS_BYTE_BUFFER

#include "pifus_identifiers.h"
#include "stdbool.h"
#include "stddef.h"

struct pifus_byte_buffer {
  uint16_t head;
  uint16_t end;
  uint16_t tail;
};

/**
 * @brief Creates a new byte buffer.
 *
 * @param byte_buffer Ptr to the byte buffer struct.
 * @param buffer_length Length of the underlying array.
 */
void pifus_byte_buffer_create(struct pifus_byte_buffer *byte_buffer,
                              uint16_t buffer_length);

/**
 * @brief Adds bytes to a byte buffer.
 *
 * @param byte_buffer Ptr to the byte buffer struct.
 * @param buffer Ptr to buffer.
 * @param to_add Byte array that should be added to the byte buffer
 * @param to_add_length Length of the to_add array
 *
 * @return The amount of bytes that could be stored.
 */
uint16_t pifus_byte_buffer_add(struct pifus_byte_buffer *byte_buffer,
                               uint8_t *buffer, uint8_t *to_add,
                               uint16_t to_add_length);

/**
 * @brief Pop bytes from a byte buffer.
 *
 * @param byte_buffer Ptr to the byte buffer struct.
 * @param buffer Ptr to buffer.
 * @param output_buffer Byte array that should be added to the byte buffer
 * @param to_add_length Length of the to_add array
 *
 * @return The amount of bytes that could be popped.
 */
uint16_t pifus_byte_buffer_pop(struct pifus_byte_buffer *byte_buffer,
                               uint8_t *buffer, uint16_t length,
                               uint8_t *output_buffer);

/**
 * @brief Check if byte buffer is empty.
 *
 * @param byte_buffer Ptr to the byte buffer struct.
 * @return True if buffer is empty, else false.
 */
uint16_t pifus_byte_buffer_is_empty(struct pifus_byte_buffer *byte_buffer);

#endif