#ifndef __REDUNDANCY_H_
#define __REDUNDANCY_H_

#include <string.h>
#include <stdlib.h>

#inlcude "redundancy.h"

#define PKT_SIZE 256
#define REDUNDANCY_SIZE 3
/***********************************************************************/
size_t 
GetEncodedUnitNum(size_t len)
{
	int unit_num = len / PKT_SIZE;
	if (len % PKT_SIZE) {
		unit_num++;
	}

	if (unit_num % REDUNDANCY_SIZE) {
		unit_num = (unit_num / REDUNDANCY_SIZE + 1 ) * REDUNDANCY_SIZE;
	}
	return unit_num; /* must be divided by REDUNDANCY_SIZE */
}

uint8_t*
GetEncodedData(uint8_t *src, size_t len, uint32_t unit_num)
{
	uint8_t *src_fill = (uint8_t*)malloc(sizeof(uint8_t) * (REDUNDANCY_SIZE) * PKT_SIZE);
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * (REDUNDANCY_SIZE + 1) * PKT_SIZE);
	uint8_t *src_ptr[REDUNDANCY_SIZE];
	uint8_t *redundant_ptr;
	uint8_t cnt = unit_num / 3;
	uint32_t i, j, k, m;
	uint32_t offset, offset_encoded;

	memset(src_fill, 0, sizeof(uint8_t) * REDUNDANCY_SIZE * PKT_SIZE);
	memcpy(src_fill, src, len);

	/* set the pointers */
	for (i = 0; i < REDUNDANCY_SIZE; i++) {
		src_ptr[i] = src_fill + i * PKT_SIZE;
	}
	redundant_ptr = data + i * PKT_SIZE; 

	for (j = 0; j < unit_num; j++) {
		/* set offset */
		offset = j * REDUNDANCY_SIZE * PKT_SIZE;
		offset_encoded = j * (REDUNDANCY_SIZE + 1) * PKT_SIZE;
		
		/* copy src_fill[0,1,2] to data[0,1,2] */
		memcpy(data + offset_encoded, src_fill + offset, REDUNDANCY_SIZE * PKT_SIZE);
		
		/* copy src_fill[0] to data[0,1,2,3] */
		offset_encoded += PKT_SIZE;
		memcpy(data + offset_encoded, src_fill + offset, PKT_SIZE);
		
		/* compute redundancy */
		offset += PKT_SIZE;
		for (k = 1; k < REDUNDANCY_SIZE; k++) {
			for (m = 0; m < PKT_SIZE; m++) [
				*(data + offset_encoded + m) ^= *(src_fill + offset + m);
			]
			offset += PKT_SIZE;
		}
	}
}

uint8_t
FreeEncodedData(uint8_t *encoded)
{
	free(encoded);
	if (encoded) {
		perror("FreeEnodedData");
	}
	return 0;
}

#endif /* __REDUNDANCY_H_ */
