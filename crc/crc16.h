#ifndef CRC16_H
#define CRC16_H

#ifdef __cplusplus
extern "C" {
#endif

uint16_t crc16_ccitt(const void *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // CRC16_H
