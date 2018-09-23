#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf          printf
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE

#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include <string.h>
#include <unistd.h>

#define DEV_RANDOM_THRESHOLD        32


#define DFL_EC_CURVE            0
#define USAGE_DEV_RANDOM ""

#define FORMAT_PEM              0
#define FORMAT_DER              1

#define DFL_TYPE                MBEDTLS_PK_RSA
#define DFL_RSA_KEYSIZE         2048
#define DFL_FILENAME            "/spiffs/keyfile.key"
#define DFL_FORMAT              FORMAT_PEM
#define DFL_USE_DEV_RANDOM      0

#define USAGE \
    "\n usage: gen_key param=<>...\n"                   \
    "\n acceptable parameters:\n"                       \
    "    type=rsa|ec           default: rsa\n"          \
    "    rsa_keysize=%%d        default: 4096\n"        \
    "    ec_curve=%%s           see below\n"            \
    "    filename=%%s           default: keyfile.key\n" \
    "    format=pem|der        default: pem\n"          \
    USAGE_DEV_RANDOM                                    \
    "\n"

/*
 * global options
 */
struct options
{
    mbedtls_pk_type_t type;                   /* the type of key to generate          */
    int rsa_keysize;            /* length of key in bits                */
    int ec_curve;               /* curve identifier for EC keys         */
    const char *filename;       /* filename of the key file             */
    int format;                 /* the output format to use             */
    int use_dev_random;         /* use /dev/random as entropy source    */
} opt;

static int write_private_key( mbedtls_pk_context *key, const char *output_file )
{
    int ret;
    FILE *f;
    unsigned char output_buf[4000];
    unsigned char *c = output_buf;
    size_t len = 0;

    memset(output_buf, 0, 4000);
    if( opt.format == FORMAT_PEM )
    {
        if( ( ret = mbedtls_pk_write_key_pem( key, output_buf, 4000 ) ) != 0 )
            return( ret );

        len = strlen( (char *) output_buf );
    }
    else
    {
        if( ( ret = mbedtls_pk_write_key_der( key, output_buf, 4000 ) ) < 0 )
            return( ret );

        len = ret;
        c = output_buf + sizeof(output_buf) - len;
    }

    if( ( f = fopen( output_file, "wb" ) ) == NULL )
        return( -1 );

    if( fwrite( c, 1, len, f ) != len )
    {
        fclose( f );
        return( -1 );
    }

    fclose( f );

    return( 0 );
}

void keySetup(void*)
{
    printf("in key setup!\n");
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_pk_context key;
    char buf[1024];
    int i;
    char *p, *q;
    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "gen_key";
    printf("initializing variables...\n");
 
    /*
     * Set to sane values
     */

    mbedtls_mpi_init( &N ); mbedtls_mpi_init( &P ); mbedtls_mpi_init( &Q );
    mbedtls_mpi_init( &D ); mbedtls_mpi_init( &E ); mbedtls_mpi_init( &DP );
    mbedtls_mpi_init( &DQ ); mbedtls_mpi_init( &QP );

    mbedtls_pk_init( &key );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    memset( buf, 0, sizeof( buf ) );

    opt.type                = DFL_TYPE;
    opt.rsa_keysize         = DFL_RSA_KEYSIZE;
    opt.ec_curve            = DFL_EC_CURVE;
    opt.filename            = DFL_FILENAME;
    opt.format              = DFL_FORMAT;
    opt.use_dev_random      = DFL_USE_DEV_RANDOM;

    mbedtls_printf( "\n  . Seeding the random number generator..." );
    fflush( stdout );

    mbedtls_entropy_init( &entropy );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned -0x%04x\n", -ret );
        goto exit;
    }

    /*
     * 1.1. Generate the key
     */
    mbedtls_printf( "\n  . Generating the private key ..." );
    fflush( stdout );

    if( ( ret = mbedtls_pk_setup( &key, mbedtls_pk_info_from_type( opt.type ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_pk_setup returned -0x%04x", -ret );
        goto exit;
    }

    if( opt.type == MBEDTLS_PK_RSA )
    {
        ret = mbedtls_rsa_gen_key( mbedtls_pk_rsa( key ), mbedtls_ctr_drbg_random, &ctr_drbg,
                                   opt.rsa_keysize, 65537 );
        if( ret != 0 )
        {
            mbedtls_printf( " failed\n  !  mbedtls_rsa_gen_key returned -0x%04x", -ret );
            goto exit;
        }
    }
    else
    {
        mbedtls_printf( " failed\n  !  key type not supported\n" );
        goto exit;
    }

    /*
     * 1.2 Print the key
     */
    mbedtls_printf( " ok\n  . Key information:\n" );

    if( mbedtls_pk_get_type( &key ) == MBEDTLS_PK_RSA )
    {
        mbedtls_rsa_context *rsa = mbedtls_pk_rsa( key );

        if( ( ret = mbedtls_rsa_export    ( rsa, &N, &P, &Q, &D, &E ) ) != 0 ||
            ( ret = mbedtls_rsa_export_crt( rsa, &DP, &DQ, &QP ) )      != 0 )
        {
            mbedtls_printf( " failed\n  ! could not export RSA parameters\n\n" );
            goto exit;
        }

        mbedtls_mpi_write_file( "N:  ",  &N,  16, NULL );
        mbedtls_mpi_write_file( "E:  ",  &E,  16, NULL );
        mbedtls_mpi_write_file( "D:  ",  &D,  16, NULL );
        mbedtls_mpi_write_file( "P:  ",  &P,  16, NULL );
        mbedtls_mpi_write_file( "Q:  ",  &Q,  16, NULL );
        mbedtls_mpi_write_file( "DP: ",  &DP, 16, NULL );
        mbedtls_mpi_write_file( "DQ:  ", &DQ, 16, NULL );
        mbedtls_mpi_write_file( "QP:  ", &QP, 16, NULL );
    }
    else
        mbedtls_printf("  ! key type not supported\n");

    /*
     * 1.3 Export key
     */
    mbedtls_printf( "  . Writing key to file..." );

    if( ( ret = write_private_key( &key, opt.filename ) ) != 0 )
    {
        mbedtls_printf( " failed\n" );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    exit_code = MBEDTLS_EXIT_SUCCESS;

exit:

    if( exit_code != MBEDTLS_EXIT_SUCCESS )
    {
        mbedtls_strerror( ret, buf, sizeof( buf ) );
        mbedtls_printf( " - %s\n", buf );
    }

    mbedtls_mpi_free( &N ); mbedtls_mpi_free( &P ); mbedtls_mpi_free( &Q );
    mbedtls_mpi_free( &D ); mbedtls_mpi_free( &E ); mbedtls_mpi_free( &DP );
    mbedtls_mpi_free( &DQ ); mbedtls_mpi_free( &QP );

    mbedtls_pk_free( &key );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    vTaskDelete(NULL);
    // return;
}