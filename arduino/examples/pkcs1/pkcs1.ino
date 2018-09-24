#include <stdio.h>
#include <stdlib.h>
#define mbedtls_snprintf        snprintf
#define mbedtls_printf          printf
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE

#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/md.h"
#include "mbedtls/pk.h"

#include <stdarg.h>
#include "FS.h"
#include "SPIFFS.h"
#include "mbedtls/sha256.h"
#define FORMAT_SPIFFS_IF_FAILED true


void setup() {
    
    Serial.begin(115200);
    delay(1000);
    printf("hello world\n");
    const unsigned char input[] = "Hello World!";

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    printf("spiffs ready");
    size_t len;
    mbedtls_pk_context pk;
    mbedtls_pk_init( &pk );
    // xTaskCreatePinnedToCore(keySetup, "keySetup", 16384, NULL, 1, NULL, 0);
    // return;

    // keySetup();
    printf("key setup done");
    int ret;
    delay(1000);
    if( ( ret = mbedtls_pk_parse_keyfile( &pk, "/spiffs/keyfile.key", "" ) ) != 0 )
    {
        printf( " failed\n  ! Could not parse '%s'\n", "/spiffs/keyfile.key" );
        //goto exit;
    }
    unsigned char hash[32];
    mbedtls_sha256_ret(input, strlen((char*)input), hash, 0);
    printf("Here is the sha256:\n");
    for (int i = 0; i < 32; i++)
    {
        printf("%02X", hash[i]);
    }
    printf("\n");

    FILE *f;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
    const char *pers = "mbedtls_pk_sign";
    size_t olen = 0;

    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    mbedtls_printf( "\n  . Seeding the random number generator..." );
    fflush( stdout );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned -0x%04x\n", -ret );
        goto exit;
    }

    mbedtls_printf( "\n  . Generating the SHA-256 signature" );
    fflush( stdout );

    if( ( ret = mbedtls_pk_sign( &pk, MBEDTLS_MD_SHA256, hash, 0, buf, &olen,
                         mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_pk_sign returned -0x%04x\n", -ret );
        goto exit;
    }

    printf("\n\nHere is the signature:\n");
    for (int i = 0; i < olen; i++)
    {
        printf("%02X", buf[i]);
    }
    printf("\n");

    mbedtls_printf( "\n  . Verifying the SHA-256 signature" );
    fflush( stdout );

    if( ( ret = mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256, hash, 0,
                           buf, olen ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_pk_verify returned -0x%04x\n", -ret );
        goto exit;
    }

    mbedtls_printf( "\n  . OK (the signature is valid)\n\n" );

exit:
    mbedtls_pk_free( &pk );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
}

void loop() {
    delay(1000);
    printf(".");
}
