#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include "common.h"

// function to assign values from args to structure variables                                                                                                                                                                                                                    
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
//function to open file
Status open_file(DecodeInfo *decInfo){
    // open stego image in read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");

    // true if file pointer is null
    if (decInfo->fptr_stego_image == NULL){
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

    	return e_failure;
    }

    // open output file in write mode
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    // true if file pointer is null
    if (decInfo->fptr_secret == NULL){
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

    	return e_failure;
    }

    return e_success;
}

// function to convert int to binary for debugging purpose
char* toBinary(int n, int len){   
    // len in bits
    char* binary = (char*)malloc(sizeof(char) * len);
    int k = 0;  
    for (unsigned i = (1 << len - 1); i > 0; i = i / 2) {
        binary[k++] = (n & i) ? '1' : '0';
    }
    binary[k] = '\0';
    return binary;
}
// extract 8 lsbs from 8 bytes of image data, combine them and return corresponding character
char decode_lsb_from_byte(char *image_data){
    char data = 0 ;
    for( int i = 0 ; i < 8 ; i++){
        data = (data | (( image_data[i] & 1 ) << (7-i))); 
    }
    return data;
}
// function to decode magic string from image
Status decode_magic_string(FILE *fptr_stego_image, DecodeInfo *decInfo){

    //set file pointer at the end of header data
    fseek(fptr_stego_image,54,SEEK_SET);

    char magic_string[2];
    
    for( int i = 0 ; i < 2 ; i++){
        fread(decInfo->image_data,sizeof(char),8,fptr_stego_image);
        magic_string[i] = decode_lsb_from_byte(decInfo->image_data);
    }
    printf("Decoded Magic String Successfully\n");
    printf("Enter the magic string : \n> ");
    //scan magic string from user
    scanf("%s",decInfo->magic_string);

    // if(strcmp(magic_string,MAGIC_STRING)==0)

    // true if magic string entered and magic string decoded both match
    if(strcmp(magic_string,decInfo->magic_string)==0){
        return e_success;
    }else{
        printf("Magic Failed : Authentication FAILURE\n");
        return e_failure;
    }
}

//input 32 bytes of data out of which 32 bits (lsb) has to be extracted forming 4 byte data
int decode_lsb_from_int(char *image_data){
    int size = 0 ;
    for( int i = 0 ; i < 32 ; i++){
        size = (size | (( image_data[i] & 1 ) << (31-i))); 
    }
    return size;
}

//function to decode extension file size ( 4byte data )
Status decode_extension_size(DecodeInfo *decInfo){
    
    char image_data[32];
    // extracting 32 bytes of data into 32 byte array 
    fread(image_data,sizeof(char),32,decInfo->fptr_stego_image);
    // passing image data to extract size
    decInfo->secret_file_extension_size = decode_lsb_from_int(image_data);
    return e_success;
}

// function to decode extension
Status decode_extension(DecodeInfo *decInfo){
    //declare char array of size previously decoded
    char file_ext[decInfo->secret_file_extension_size];
    char image_data[decInfo->secret_file_extension_size];
    // run loops from iterations = number of chars 
    for(int i = 0 ; i < decInfo->secret_file_extension_size ; i++){
        // extract in set of 8 bytes to get 8 bit char data from lsb
        fread(image_data,sizeof(char),8,decInfo->fptr_stego_image);
        // store each character obtained
        file_ext[i]=decode_lsb_from_byte(image_data);
    }
    return e_success;
}

// function to decode secret file size (4 byte int data )
Status decode_secret_file_size(DecodeInfo *decInfo){
    // 32 byte array to store integer size
    char image_data[32];
    // extract 32 bytes of data from stego image
    fread(image_data,sizeof(char),32,decInfo->fptr_stego_image);
    // get secret data from function
    decInfo->size_secret_file = decode_lsb_from_int(image_data);
    return e_success;
}
// function to decode secret data
Status decode_secret_data(DecodeInfo *decInfo){

    char ch;
    // for loop to iterate for secret file size number of times
    for(int i=0;i<decInfo->size_secret_file;i++){
        // read 8 bytes of data from image to get 8 bits ( 1 byte of char data )
        fread(decInfo->image_data,sizeof(char),8,decInfo->fptr_stego_image);
        // get the corresponding char
        ch=decode_lsb_from_byte(decInfo->image_data);
        // write the char to the output file
        fwrite(&ch,sizeof(char),1,decInfo->fptr_secret);
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo){
    if(open_file(decInfo)==e_success){
        printf("File Opened Successfully\n");
        printf("Started Decoding...\n");
        if(decode_magic_string(decInfo->fptr_stego_image,decInfo)==e_success){
            printf("Magic String Matched !!\n");
            if(decode_extension_size(decInfo)==e_success){
                printf("Decoded extension size successfully.\n");
                if(decode_extension(decInfo)==e_success){
                    printf("Decoded extension succesfully\n");
                    if(decode_secret_file_size(decInfo)==e_success){
                        printf("Decoded file size successfully\n");
                        if(decode_secret_data(decInfo)==e_success){
                            return e_success;
                        }else{
                            printf("ERROR : SECRET DATA EXTRACTION FAILURE.\n");
                                    return e_failure;
                        }
                    }else{
                        printf("ERROR : DECODING FILE SIZE FAILED.\n");
                                return e_failure;
                    }
                }else{
                    printf("ERROR : DECODING EXTENSION FAILED.\n");
                            return e_failure;
                }
            }else{
                printf("ERROR : DECODING EXTENSION SIZE FAILED.\n");
                        return e_failure;
            }
        }else{
            printf("ERROR : DECODING FAILED\n");
                    return e_failure;
        }
    }else{
        printf("ERROR : FILE OPENING FAILED\n");
        return e_failure;
    }
}