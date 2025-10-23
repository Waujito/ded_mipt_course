
#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t hash_crc32(const uint8_t data[], size_t data_length);

uint32_t hash_crc32_add(uint32_t base_hash, 
			const uint8_t data[], size_t data_length);

#ifdef __cplusplus
}
#endif

#endif /* HASH_H */

