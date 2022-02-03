#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include "common.h"

/* Function Definitions */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo){
    if(strcmp(strstr(argv[2],"."),".bmp") == 0){
        encInfo->src_image_fname = argv[2];
    }else{
        return e_failure;
    }
    if(strcmp(strstr(argv[3],"."),".txt") == 0){
        encInfo->secret_fname = argv[3];
    }else{
        return e_failure;
    }
    if(argv[4]!=NULL){
        encInfo->stego_image_fname = argv[4];
    }else{
        encInfo->stego_image_fname="stego.bmp";
    }
    return e_success;
}
/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
uint get_file_size(FILE *fptr){
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

//function to check if encoding is possible or not
Status check_capacity(EncodeInfo *encInfo){
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if(encInfo->image_capacity > (54 + (2+4+4+4+encInfo->size_secret_file)*8)){
        return e_success;
    }else{
        return e_failure;
    }
}
//copy the first 54 bytes of image file
Status copy_bmp_header(FILE *src , FILE *stego){
    fseek(src,0,SEEK_SET);
    char str[54];
    fread(str,sizeof(char),54,src);
    fwrite(str,sizeof(char),54,stego);
    return e_success;
}

//encode input data of 1 byte to 8 bits into lbs of 8 bytes of data
Status encode_byte_to_lsb(char data,char *image_data){
    unsigned int mask = 1 << 7 ;
    for(int i = 0 ; i < 8 ; i++ ){
        image_data[i]=(image_data[i] & 0xFE | ((data & mask)>>(7-i)));
        mask=mask>>1;
    }
}
// function to encode data of 'size' in bytes to stego image
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo){
    
    for(int i = 0 ; i < size ; i++){
        fread(encInfo->image_data,sizeof(char),8,fptr_src_image);
        encode_byte_to_lsb(data[i],encInfo->image_data);
        fwrite(encInfo->image_data,sizeof(char),8,fptr_stego_image);
    }
    return e_success;
}
// function to encode magic string
Status encode_magic_string(const char *magic_string,EncodeInfo *encInfo){
    encode_data_to_image(magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo);
    return e_success;
}

// function to encode size to lsb 32 bits of int data into 32 bytes of image data
Status encode_size_to_lsb(char *str, int size){
    unsigned int mask = 1 << 31;
    for(int i = 0 ; i < 32 ; i ++){
        str[i] = (str[i] & 0xFE | ((size & mask) >> (31-i)));
        mask = mask>>1;
    }
}

// function to encode size into the stogo image 
Status encode_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image){
    char str[32];
    fread(str,sizeof(char),32,fptr_src_image);
    encode_size_to_lsb(str,size);
    fwrite(str,sizeof(char),32,fptr_stego_image);
    return e_success;
}

// function to encode secret file extension 
Status encode_secret_file_extn(const char *file_ext,EncodeInfo *encInfo){
    file_ext = ".txt";
    // pass text, length of text to encode into image
    encode_data_to_image(file_ext,strlen(file_ext),encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo);
    return e_success;
}
// function to encode secret file size which is 32 bit int data to be stored in 32 bytes of image data lsb
Status encode_secret_file_size(long file_size,EncodeInfo *encInfo){
    char str[32];
    fread(str,sizeof(char),32,encInfo->fptr_src_image);
    encode_size_to_lsb(str,file_size);
    fwrite(str,sizeof(char),32,encInfo->fptr_stego_image);
    return e_success;
}
//encode secret file data into the stego image
Status encode_secret_file_data(EncodeInfo *encInfo){
    char ch;
    fseek(encInfo->fptr_secret,0,SEEK_SET);
    for(int i=0;i<encInfo->size_secret_file;i++){
        // take 8 bytes of image data
        fread(encInfo->image_data,sizeof(char),8,encInfo->fptr_src_image);
        // take 1 char from secret text file
        fread(&ch,sizeof(char),1,encInfo->fptr_secret);
        //encode 8 bits of char into 8 bytes of image data
        encode_byte_to_lsb(ch,encInfo->image_data);
        // write image data to stego image
        fwrite(encInfo->image_data,sizeof(char),8,encInfo->fptr_stego_image);   
    }
    return e_success;
}
// function to copy remaining image data
Status copy_remaining_img_data(FILE *fptr_src_img,FILE *fptr_stego_img){

    char ch;
    // while freas is greater than zero or reads a valid pointer
    while(fread(&ch,1,1,fptr_src_img)>0){
        // write remiaining data to stego image
        fwrite(&ch,1,1,fptr_stego_img);
    }
    return e_success;
}
// primary encoding function
Status do_encoding(EncodeInfo *encInfo){
    if(open_files(encInfo)==e_success){
        printf("Opened File Successfully.\n");
        printf("Started Encoding...\n");
        if(check_capacity(encInfo)==e_success){
            printf("Encoding the secret data...\n");
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success){
                printf("Copied Header Successfully.\n");    
                if(encode_magic_string(MAGIC_STRING,encInfo)== e_success){
                    printf("Encoded Magic String Successfully.\n");
                    if(encode_size(strlen(".txt"),encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success){
                        printf("Encoded secret file extension size successfully.\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_success){
                            printf("Encoded secret file extension successfully.\n");
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success){
                                printf("Encoded secret file size Successflly.\n");
                                if(encode_secret_file_data(encInfo)==e_success){
                                    printf("Encoded secret data successfully ;)\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success){
                                        printf("Copied remaining data successfully.\n");
                                        return e_success;
                                    }else{
                                        printf("COPYING FAILED : REMAINING IMAGE.\n");
                                        return e_failure;
                                    }
                                }else{
                                    printf("Encoding FAILED : SECRET FILE DATA\n");
                                    return e_failure;
                                }
                            }else{
                                printf("Encoding FAILED : SECRET FILE SIZE\n");
                                return e_failure;
                            }
                        }else{
                            printf("Encoding FAILED : EXTENSION\n");
                            return e_failure;
                        }
                    }else{
                        printf("Encoding FAILED : EXTENSION SIZE\n");
                        return e_failure;
                    }
                }else{
                    printf("Encoding FAILED : MAGIC STRING\n");
                    return e_failure;
                }         
            }else{
                printf("Copying header files FAILED.");
                return e_failure;
            }
        }else{
            printf("Encoding not possible.\n");
            return e_failure;
        }
    }else{
        printf("File Opening Unsuccessfull\n");
        return e_failure;
    }
}