//:folding=explicit:

//{{{ License
/*
*   This file is part of gmp3view.
*   (c) 2006-2011 Malte Rohde
*
*   gmp3view is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   gmp3view is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with gmp3view; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//}}}

using namespace std;
#include <iostream>
#include <string>
#include <jpeglib.h>
#include "imageio/jmem_dest.h"
#include "imageio/jmem_src.h"
#include "jpeg.h"

extern size_t bytes_written;

int jpeg_compress (unsigned char *src, unsigned char *dst, size_t size)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride = 600;
	JSAMPROW row_pointer[1];
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	/* Image description */
	cinfo.image_width = 200;
	cinfo.image_height = 200;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	
	/* Set defaults, then non-default parameters */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 85, TRUE);

	/* Set destination */
	jpeg_memory_dest(&cinfo, dst, size);
	
	/* Compress */
	jpeg_start_compress(&cinfo, TRUE);
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &src[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);		
	}	
	jpeg_finish_compress(&cinfo);
	
	jpeg_destroy_compress(&cinfo);
	
	return bytes_written;
}

unsigned char *jpeg_decompress(const unsigned char *src, size_t n_bytes)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;	
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	/* Set the source */
	jpeg_memory_src(&cinfo, src, n_bytes);
	
	/* This somehow does not work */
	jpeg_read_header(&cinfo, TRUE);
	/* Therefore we set these values explicitly */
	cinfo.image_width = 200;
	cinfo.image_height = 200;
	cinfo.output_height = 200;
	cinfo.output_width = 200;
	cinfo.output_components = 3;
	row_stride = cinfo.output_width * cinfo.output_components;
	
	unsigned char *dst = new unsigned char[200 * 200 * 3];	

	buffer = cinfo.mem->alloc_sarray((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		
	jpeg_start_decompress(&cinfo);
	while (cinfo.output_scanline < cinfo.output_height) {	

		jpeg_read_scanlines(&cinfo, buffer, 1);

		for(int i = 0; i < row_stride; i++) {
			dst[(cinfo.output_scanline - 1) * row_stride + i] = (unsigned char)buffer[0][i];
		}
	}
	jpeg_finish_decompress(&cinfo);
	
	jpeg_destroy_decompress(&cinfo);
	
	return dst;
}
	
	
