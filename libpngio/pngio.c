/*
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <png.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

static void abort_(const char * s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void write_png_file(unsigned char *data, int width, int height, size_t bpp, const char *filename) {
    png_bytep *row_pointers = malloc(height * sizeof *row_pointers);

    for (int y = 0; y < height; y++) {
        row_pointers[y] = data + (y * width * bpp);
    }

    png_byte color_type = (bpp == 4 ? PNG_COLOR_TYPE_RGB_ALPHA
                           : (bpp == 3 ? PNG_COLOR_TYPE_RGB : 0));
    png_byte bit_depth = 8;

    png_structp png_ptr;
    png_infop info_ptr;

    /* create file */
    FILE *fp = fopen(filename, "wb");
    if (!fp)
        abort_("[write_png_file] File %s could not be opened for writing", filename);


    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[write_png_file] png_create_write_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[write_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during init_io");

    png_init_io(png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing header");

    png_set_IHDR(png_ptr, info_ptr, width, height,
            bit_depth, color_type, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing bytes");

    png_write_image(png_ptr, row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during end of write");

    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation
    for (int y=0; y<height; y++) {
        free(row_pointers[y]);
    } */
    png_destroy_info_struct(png_ptr, &info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    free(row_pointers);

    fclose(fp);
}

