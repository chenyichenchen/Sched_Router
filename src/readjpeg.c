#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <jerror.h>
#include <string.h>
#include "readjpeg.h"

int readjpeg(char *filename,char *dst,int *width,int *height,int *components)
{	
	struct jpeg_decompress_struct cinfo;/* declaration for jpeg decompression */
	struct jpeg_error_mgr jerr;
	FILE *infile;
	unsigned char *buffer;
	unsigned long pos,buffersize;
	infile=fopen(filename,"rb");/* open input jpeg file */
	cinfo.err=jpeg_std_error(&jerr);/* init jpeg decompress object error handler */
	jpeg_create_decompress(&cinfo);/* bind jpeg decompress object to infile */	
	jpeg_stdio_src(&cinfo, infile);	
	jpeg_read_header(&cinfo, TRUE);/* read jpeg header */
	jpeg_start_decompress(&cinfo);
	*width=cinfo.output_width; *height=cinfo.output_height; *components=cinfo.output_components;
	buffersize=(*width)*(*components);
	buffer=(unsigned char *)malloc(buffersize);
	pos=0;
	while(cinfo.output_scanline<cinfo.output_height){
		jpeg_read_scanlines(&cinfo,&buffer,1);
		memcpy(dst+pos,buffer,buffersize);
		pos+=buffersize;}					// next scanline	
	jpeg_finish_decompress(&cinfo);/* finish decompress, destroy decompress object */
	jpeg_destroy_decompress(&cinfo);
	free(buffer);/* release memory buffer */
	fclose(infile);/* close jpeg inputing file */
	return 0;
}
