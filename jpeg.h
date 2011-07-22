#ifndef __JPEG_H__
#define __JPEG_H__

int jpeg_compress(unsigned char *src, unsigned char *dst, size_t size);
unsigned char *jpeg_decompress(const unsigned char *src, size_t n_bytes);

#endif /* __JPEG_H__ */
