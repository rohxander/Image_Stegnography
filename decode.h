#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types
#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo{
    uint image_capacity;
    uint bits_per_pixel;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    int secret_file_extension_size;
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    int size_secret_file;
    char *magic_string;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
} DecodeInfo;

//function to assign values from args to structure variables                                                                                                                                                                                                                    
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

//primary function
Status do_decoding(DecodeInfo *decInfo);

// function to decode magic string from image
Status decode_magic_string(FILE *fptr_stego_image, DecodeInfo *decInfo);

//function to open file
Status open_file(DecodeInfo *decInfo);

// function to convert int to binary for debugging purpose
char* toBinary(int n, int len);

//input 32 bytes of data out of which 32 bits (lsb) has to be extracted forming 4 byte data
int decode_lsb_from_int(char *image_data);

//function to decode extension file size ( 4byte data )
Status decode_extension_size(DecodeInfo *decInfo);

// function to decode extension
Status decode_extension(DecodeInfo *decInfo);

// function to decode secret file size (4 byte int data )
Status decode_secret_file_size(DecodeInfo *decInfo);

// function to decode secret data
Status decode_secret_data(DecodeInfo *decInfo);
#endif