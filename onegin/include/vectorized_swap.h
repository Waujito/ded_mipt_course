#ifndef VECTORIZED_SWAP_H
#define VECTORIZED_SWAP_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define ES_SWAP_BYTES_(p1, p2, dtype) {		\
	dtype dt1 = 0;				\
	memcpy(&dt1, p1, sizeof(dt1));		\
	memcpy(p1, p2, sizeof(dt1));		\
	memcpy(p2, &dt1, sizeof(dt1));		\
	p1 += sizeof(dt1);			\
	p2 += sizeof(dt1);			\
}

static int vectorized_swap(char *p1, char *p2, size_t sz) {
	assert (p1);
	assert (p2);

	size_t processed_sz = 0;

	while (processed_sz + sizeof(uint64_t) <= sz) {
		ES_SWAP_BYTES_(p1, p2, uint64_t);
		processed_sz += sizeof(uint64_t);
	}

	if (processed_sz + sizeof(uint32_t) <= sz) {
		ES_SWAP_BYTES_(p1, p2, uint32_t);
		processed_sz += sizeof(uint32_t);
	}

	if (processed_sz + sizeof(uint16_t) <= sz) {
		ES_SWAP_BYTES_(p1, p2, uint16_t);
		processed_sz += sizeof(uint16_t);
	}

	if (processed_sz + sizeof(char) <= sz) {
		ES_SWAP_BYTES_(p1, p2, char);
		processed_sz += sizeof(char);
	}

	return 0;
}

#undef ES_SWAP_BYTES_

#endif /* VECTORIZED_SWAP_H */
