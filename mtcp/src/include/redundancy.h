#ifndef __REDUNDANCY_H_
#define __REDUNDANCY_H_

#include <string.h>
#include <stdlib.h>

#define PKT_SIZE 256
#define REDUNDANCY_SIZE 3
/***********************************************************************/
size_t 
GetEncodedUnitNum(size_t len);

uint8_t*
GetEncodedData(uint8_t *src, size_t len, uint32_t unit_num);

uint8_t
FreeEncodedData(uint8_t *encoded);

uint8_t
GetLostIndex()

uint8_t*
GetDecodedData(uint8_*src, size_t len, uint32_t unit_num);

#endif /* __REDUNDANCY_H_ */
