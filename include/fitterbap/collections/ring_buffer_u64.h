/*
 * Copyright 2020-2021 Jetperch LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 *
 * @brief Ring buffer for u64 data values.
 */

#ifndef FBP_COLLECTIONS_RING_BUFFER_U64_H__
#define FBP_COLLECTIONS_RING_BUFFER_U64_H__

#include "fitterbap/common_header.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @ingroup fbp_collections
 * @defgroup fbp_collections_rbu64 Ring buffer for u64 data values
 *
 * @brief Provide a simple, fast u64 FIFO buffer.
 *
 * @{
 */

FBP_CPP_GUARD_START

/// The ring buffer containing unsigned 64-bit integers.
struct fbp_rbu64_s {
    uint32_t head;
    uint32_t tail;
    uint64_t * buf;
    uint32_t buf_size;  // Size of buf in u64, capacity = buf_size - 1.
};

/**
 * @brief Clear the buffer and discard all data.
 *
 * @param self The buffer instance.
 */
static inline void fbp_rbu64_clear(struct fbp_rbu64_s * self) {
    if (self->tail >= self->buf_size) {
        self->head = 0;
        self->tail = 0;
    } else {
        self->tail = self->head;
    }
}

/**
 * @brief Initialize the buffer instance.
 *
 * @param self The buffer instance.
 * @param buffer The underlying buffer to use, which must remain valid.
 * @param buffer_size The size of buffer in u64.
 */
static inline void fbp_rbu64_init(struct fbp_rbu64_s * self, uint64_t * buffer, uint32_t buffer_size) {
    self->buf = buffer;
    self->buf_size = buffer_size;
    self->head = 0;
    self->tail = 0;
}

static inline uint32_t fbp_rbu64_size(struct fbp_rbu64_s * self) {
    uint32_t sz = ((self->head + self->buf_size) - self->tail);
    if (sz >= self->buf_size) {
        sz -= self->buf_size;
    }
    return sz;
}

static inline bool fbp_rbu64_is_empty(struct fbp_rbu64_s * self) {
    return (self->head == self->tail);
}

static inline uint32_t fbp_rbu64_empty_size(struct fbp_rbu64_s * self) {
    return self->buf_size - 1 - fbp_rbu64_size(self);
}

static inline uint32_t fbp_rbu64_capacity(struct fbp_rbu64_s * self) {
    return (self->buf_size - 1);
}

static inline uint64_t * fbp_rbu64_head(struct fbp_rbu64_s * self) {
    return (self->buf + self->head);
}

static inline uint64_t * fbp_rbu64_tail(struct fbp_rbu64_s * self) {
    return (self->buf + self->tail);
}

static inline uint32_t fbp_rbu64_offset_incr(struct fbp_rbu64_s * self, uint32_t offset) {
    uint32_t next_offset = offset + 1;
    if (next_offset >= self->buf_size) {
        next_offset = 0;
    }
    return next_offset;
}

static inline bool fbp_rbu64_push(struct fbp_rbu64_s * self, uint64_t value) {
    uint32_t head = self->head;
    uint32_t next_head = fbp_rbu64_offset_incr(self, head);
    if (next_head == self->tail) {  // full
        return false;
    }
    self->buf[head] = value;
    self->head = next_head;
    return true;
}

static inline bool fbp_rbu64_pop(struct fbp_rbu64_s * self, uint64_t * value) {
    uint32_t tail = self->tail;
    if (self->head == tail) {  // empty
        return false;
    }
    *value = self->buf[tail];
    self->tail = fbp_rbu64_offset_incr(self, tail);
    return true;
}

static inline bool fbp_rbu64_add(struct fbp_rbu64_s * self, uint64_t const * buffer, uint32_t count) {
    if (count > fbp_rbu64_empty_size(self)) {
        return false;
    }
    if ((self->head + count) >= self->buf_size) {
        uint32_t sz = self->buf_size - self->head;
        fbp_memcpy(fbp_rbu64_head(self), buffer, sz * sizeof(*buffer));
        self->head = 0;
        buffer += sz;
        count -= sz;
    }
    if (count) {
        fbp_memcpy(fbp_rbu64_head(self), buffer, count * sizeof(*buffer));
        self->head += count;
    }
    return true;
}

static inline bool fbp_rbu64_discard(struct fbp_rbu64_s * self, uint32_t count) {
    if (count > fbp_rbu64_size(self)) {
        self->tail = self->head;
        return false;
    }
    uint32_t tail = self->tail + count;
    if (tail >= self->buf_size) {
        tail -= self->buf_size;
    }
    self->tail = tail;
    return true;
}

FBP_CPP_GUARD_END

/** @} */

#endif  /* FBP_COLLECTIONS_RING_BUFFER_U64_H__ */
