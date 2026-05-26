#include <stdlib.h>
#include <stdio.h>

/*
size_t triangle_len(size_t n) {
	size_t asize = ((1 + n) * n) / 2;
	return asize;
}
size_t triangle_idx(size_t i, size_t j) {
	if (i < j) {
		size_t t = i;
		i = j;
		j = t;
	}

	size_t idx = ((i + 1) * i) / 2 + j;
	return idx;
}
*/

struct rombus_structure {
	int *arr;

	size_t n;
};

size_t memory_count(size_t n) {
	size_t ht = n / 2;

	size_t starting_width = 2 - n % 2;
	size_t els_ct = ((starting_width + n) / 2) * (ht + n % 2);
	els_ct += ((starting_width + n - 2) / 2) * ht;

	return els_ct;
}

// -------
// ---*---
// --***--
// -*****-
// *******
// -*****-
// --***--
// ---*---
// -------
//
// --------
// ---**---
// --****--
// -******-
// ********
// ********
// -******-
// --****--
// ---**---
// --------
ssize_t index_rombus(struct rombus_structure *rombus, size_t i, size_t j) {
	size_t n = rombus->n;

	size_t ht = n / 2;
	size_t starting_width = 2 - n % 2; // 1 or 2
	
	size_t els_ct = ((starting_width + width) / 2) * (ht + height % 2);
	els_ct += ((starting_width + width - 2) / 2) * ht;



	int sqtrg =  (i <= rh) * 2 + (j <= rw);

}

/**
 * Orientation of a squared triangle inside the rectangle.
 */
// enum sqtriangle_orientation {
// 	SQO_BOTTOM_RIGHT 	= 0,
// 	SQO_BOTTOM_LEFT 	= 1,
// 	SQO_TOP_RIGHT		= 2,
// 	SQO_TOP_LEFT		= 3,
// };

// ssize_t index_rombus(struct rombus_structure *rombus, size_t i, size_t j) {
// 	size_t rh = rombus->height / 2;	
// 	size_t rw = rombus->width / 2;
//
// 	int sqtrg =  (i <= rh) * 2 + (j <= rw);
//
// }

int main() {
	
}
