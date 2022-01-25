#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

int main(int argc,char *argv[])
{
    uint img_size;
    if(check_operation_type(argv) == e_encode){
        EncodeInfo encInfo;
        printf("Selected Encoding\n");
        if(read_and_validate_encode_args(argv,&encInfo) == e_success){
            printf("Read and Validated successfully...\n");
            if(do_encoding(&encInfo) == e_success){
                printf("Completed Encoding\n");
            }else{
                printf("Failure : Encoding is not done\n");
            }
        }else{
            printf("Read and Validate unsuccessfull.\n");
        }

    }else if(check_operation_type(argv )== e_decode){
        DecodeInfo decInfo;
        printf("Selected Decoding\n");
        if(read_and_validate_decode_args(argv,&decInfo) == e_success){
            printf("Read and Validated Successfully...\n");
            if(do_decoding(&decInfo)==e_success){
                printf("Completed Decoding\n");
            }else{
                printf("ERROR : DECODING FAILED \n");
            }
        }else{
            printf("Read and Validated unsuccesfully.\n");
        }
    }
    else{
        printf("Invalid Option\nUsage:\nEncoding: ./a.out -e beautiful.bmp secret.text stego.bmp\n");
        printf("Decoding: ./a.out -d stego.bmp\n");
    }
    return 0;
}
OperationType check_operation_type(char *argv[]){
    if(strcmp(argv[1],"-e") == 0){
        return e_encode;
    }else if(strcmp(argv[1],"-d") == 0){
        return e_decode;
    }else{
        e_unsupported;
    }
}