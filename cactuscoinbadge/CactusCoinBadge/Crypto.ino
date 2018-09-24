
int setupCrypto() {
  int ret;
  const char *pers = "mbedtls_pk_sign";
  mbedtls_pk_init(&badgePK);
  mbedtls_pk_init(&nodePK);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  
  if ((ret = mbedtls_pk_parse_keyfile(&badgePK, "/spiffs/private.pem", "")) != 0) {
    Serial.printf("Failed to load badge private key, error code: -0x%04x\n", -ret);
    teardownCrypto();
    return -1;
  }
  
  if ((ret = mbedtls_pk_parse_public_keyfile(&nodePK, "/spiffs/cactuscoinapi.pem") ) != 0) {
    Serial.printf("Failed to load cactuscoinapi public key, error code: -0x%04x\n", -ret);
    teardownCrypto();
    return -1;
  }
  
  if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
  {
    Serial.printf("Failed to seed the random number generator , error code: -0x%04x\n", -ret);
    teardownCrypto();
    return -1;
  }

}

void teardownCrypto() {
  mbedtls_pk_free( &badgePK );
  mbedtls_pk_free( &nodePK );
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );
}

/*
 * Args:
 * buf - should always be allocated to a size of MBEDTLS_MPI_MAX_SIZE
 */
int sign(unsigned char *toSign, int toSignLen, unsigned char *buf, size_t *olen) {
  unsigned char hash[32];
  int ret;
  mbedtls_sha256_ret(toSign, toSignLen, hash, 0);
  
  if( ( ret = mbedtls_pk_sign( &badgePK, MBEDTLS_MD_SHA256, hash, 0, buf, olen,
                         mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
  {
    Serial.printf("Failed to sign, error code: -0x%04x\n", -ret);
    return -1;
  }
}

int verify(unsigned char *toVerify, int toVerifyLen, unsigned char *buf, size_t olen) {
  unsigned char hash[32];
  int ret;
  mbedtls_sha256_ret(toVerify, toVerifyLen, hash, 0);
  
  if( ( ret = mbedtls_pk_verify( &nodePK, MBEDTLS_MD_SHA256, hash, 0, buf, olen ) ) != 0 )
  {
    Serial.printf("Failed to verify, error code: -0x%04x\n", -ret);
    return -1;
  }
}
