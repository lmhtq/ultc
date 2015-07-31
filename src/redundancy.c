#ifndef __REDUNDANCY_H_
#define __REDUNDANCY_H_

#include <string.h>
#include <stdlib.h>

#include "tcp_stream.h"
#include "redundancy.h"

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
	uint8_t *src_fill = (uint8_t*)malloc(sizeof(uint8_t) * unit_num
			* PKT_SIZE);
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * unit_num 
			/ REDUNDANCY_SIZE * (REDUNDANCY_SIZE + 1) * PKT_SIZE);
	uint8_t *src_ptr[REDUNDANCY_SIZE];
	uint8_t *redundant_ptr;
	uint8_t cnt = unit_num / 3;
	uint32_t i, j, k, m;
	uint32_t offset, offset_encoded;

	if (src_fill == NULL) {
		perror("GetEncodedData: src_fill malloc error\n");
		exit(-1);
	}
	if (data == NULL) {
		perror("GetEncodedData: dada malloc error\n");
		exit(-1);
	}
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
		memcpy(data + offset_encoded, src_fill + offset,
				REDUNDANCY_SIZE * PKT_SIZE);
		
		/* copy src_fill[0] to data[0,1,2,3] */
		offset_encoded += PKT_SIZE;
		memcpy(data + offset_encoded, src_fill + offset, PKT_SIZE);
		
		/* compute redundancy */
		offset += PKT_SIZE;
		for (k = 1; k < REDUNDANCY_SIZE; k++) {
			for (m = 0; m < PKT_SIZE; m++) {
				*(data + offset_encoded + m) ^= 
					*(src_fill + offset + m);
			}
			offset += PKT_SIZE;
		}
	}
	return data;
}

uint8_t
FreeEncodedData(uint8_t *encoded)
{
	if (encoded == NULL) {
		free(encoded);
	}
	return 0;
}

void
AddToDecodingData(mtcp_manager_t mtcp, tcp_stream *cur_stream,
		uint32_t cur_ts, uint8_t *payload, uint32_t seq, int payloadlen)
{
	struct tcp_recv_vars *rcvvar = cur_stream->rcvvar;
	struct fragment_ctx *iter;
	struct fragment_ctx *ptr;
	struct fragment_ctx *ptr_nxt;
	uint8_t recovery[PKT_SIZE];
	uint32_t index;
	uint32_t i,j,k, num, num_nxt;
	uint32_t offset;

	if (payloadlen != PKT_SIZE || 
			(seq - rcvvar->rcvbuf->init_seq) % PKT_SIZE != 0 ) {
		perror("payloadlen error");
	}

	index = (uint32_t) (seq - rcvvar->rcvbuf->init_seq) / PKT_SIZE;
	if (index % (REDUNDANCY_SIZE + 1) == REDUNDANCY_SIZE) {
		/* find the ftcx corresponding to the redundancy */
		ptr = NULL;
		for (iter = rcvvar->rcvbuf->fctx; iter != NULL; iter = iter->next) {
			if (seq - iter->seq <= 3 * PKT_SIZE && 
					seq - iter->seq >= PKT_SIZE) {
				ptr = iter;
				ptr_nxt = ptr->next;
				break;
			}
		}
		
		num = ptr->len / PKT_SIZE;
		num_nxt = ptr_nxt->len / PKT_SIZE;
		
		memset(recovery, 0, sizeof(uint8_t) * PKT_SIZE);
		if (seq - ptr->seq == (REDUNDANCY_SIZE - 1) * PKT_SIZE &&
				ptr->len == (REDUNDANCY_SIZE - 1) * PKT_SIZE) /* Lost 0 */
		{
			for (i = 0; i < num; i++) {
				offset;
			}
				
		}
	}

}

#endif /* __REDUNDANCY_H_ */
