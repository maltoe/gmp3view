/* memio.c: routines that use the memory destination and source module
** BASED ON example.c
**/

#include <sys/types.h>
#include <stdio.h>

#include "jpeglib.h"
#include "jpegfix.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern GLOBAL(void)
write_JPEG_memory (unsigned char *img_buf, int width, int height,
                   unsigned char *jpeg_buffer, unsigned long jpeg_buffer_size,
                   int quality, unsigned long *jpeg_comp_size);

extern GLOBAL(int)
read_JPEG_memory (unsigned char *img_buf, int *width, int *height,
                  const unsigned char *jpeg_buffer,
                  unsigned long jpeg_buffer_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
