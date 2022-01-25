#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include "common.h"


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo){
    if(strcmp(strstr(argv[2],"."),".bmp") == 0){
        decInfo->stego_image_fname = argv[2];
    }else{
        return e_failure;
    }
    if(argv[3]!=NULL){
        if(strcmp(strstr(argv[3],"."),".txt") == 0){
            decInfo->secret_fname = argv[3];
        }else{
            return e_failure;
        }
    }else{
        decInfo->secret_fname="super_secret.txt";
    }
    return e_success;
}

// uint get_image_size_for_bmp(FILE *fptr_image)
// {
//     uint width, height;
//     // Seek to 18th byte
//     fseek(fptr_image, 18, SEEK_SET);

//     // Read the width (an int)
//     fread(&width, sizeof(int), 1, fptr_image);
//     printf("width = %u\n", width);

//     // Read the height (an int)
//     fread(&height, sizeof(int), 1, fptr_image);
//     printf("height = %u\n", height);

//     // Return image capacity
//     return width * height * 3;
// }

Status open_file(DecodeInfo *decInfo){
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");
    if (decInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

    	return e_failure;
    }

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    
    if (decInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

    	return e_failure;
    }
    return e_success;
}
char* toBinary(int n, int len){   
    char* binary = (char*)malloc(sizeof(char) * len);
    int k = 0;
    for (unsigned i = (1 << len - 1); i > 0; i = i / 2) {
        binary[k++] = (n & i) ? '1' : '0';
    }
    binary[k] = '\0';
    return binary;
}

char decode_lsb_from_byte(char *image_data){
    char data = 0 ;
    for( int i = 0 ; i < 8 ; i++){
        data = (data | (( image_data[i] & 1 ) << (7-i))); 
    }
    return data;
}

Status decode_magic_string(FILE *fptr_stego_image, DecodeInfo *decInfo){
    fseek(fptr_stego_image,54,SEEK_SET);
    char magic_string[2];
    for( int i = 0 ; i < 2 ; i++){
        fread(decInfo->image_data,sizeof(char),8,fptr_stego_image);
        magic_string[i] = decode_lsb_from_byte(decInfo->image_data);
    }
    if(strcmp(magic_string,MAGIC_STRING)==0){
        printf("Magic String Matched !\n");
        return e_success;
    }else{
        return e_failure;
    }
}
int decode_lsb_from_int(char *image_data){
    int size = 0 ;
    for( int i = 0 ; i < 32 ; i++){
        size = (size | (( image_data[i] & 1 ) << (31-i))); 
    }
    return size;
}
Status decode_extension_size(DecodeInfo *decInfo){
    
    char image_data[32];
    fread(image_data,sizeof(char),32,decInfo->fptr_stego_image);
    decInfo->secret_file_extension_size = decode_lsb_from_int(image_data);
    return e_success;
}
Status decode_extension(DecodeInfo *decInfo){
    char file_ext[decInfo->secret_file_extension_size];
    char image_data[decInfo->secret_file_extension_size];
    for(int i = 0 ; i < decInfo->secret_file_extension_size ; i++){
        fread(image_data,sizeof(char),8,decInfo->fptr_stego_image);
        file_ext[i]=decode_lsb_from_byte(image_data);
    }
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo){
    char image_data[32];
    fread(image_data,sizeof(char),32,decInfo->fptr_stego_image);
    decInfo->size_secret_file = decode_lsb_from_int(image_data);
    return e_success;
}

Status decode_secret_data(DecodeInfo *decInfo){
    char ch;
    for(int i=0;i<decInfo->size_secret_file;i++){
        fread(decInfo->image_data,sizeof(char),8,decInfo->fptr_stego_image);
        ch=decode_lsb_from_byte(decInfo->image_data);
        fwrite(&ch,sizeof(char),1,decInfo->fptr_secret);
    }
    return e_success;
}
Status do_decoding(DecodeInfo *decInfo){
    if(open_file(decInfo)==e_success){
        printf("File Opened Successfully\n");
        printf("Started Decoding...\n");
        if(decode_magic_string(decInfo->fptr_stego_image,decInfo)==e_success){
            printf("Decoded Magic String Successfully\n");
            if(decode_extension_size(decInfo)==e_success){
                printf("Decoded extension size successfully.\n");
                if(decode_extension(decInfo)==e_success){
                    printf("Decoded extension succesfully\n");
                    if(decode_secret_file_size(decInfo)==e_success){
                        printf("Decoded file size successfully\n");
                        if(decode_secret_data(decInfo)==e_success){
                            printf("Eureka !!\n");
                        }else{
                            printf("Oh cmOn ;(\n");
                        }
                    }else{
                        printf("ERROR : DECODING FILE SIZE FAILED.\n");
                    }
                }else{
                    printf("ERROR : DECODING EXTENSION FAILED.\n");
                }
            }else{
                printf("ERROR : DECODING EXTENSION SIZE FAILED.\n");
            }
        }else{
            printf("ERROR : DECODING FAILED\n");
        }
    }else{
        printf("ERROR : FILE OPENING FAILED\n");
        return e_failure;
    }
    return e_success;
}