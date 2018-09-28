
bool setupCrypto() {
  int ret;
  const char *pers = "mbedtls_pk_sign";
  mbedtls_pk_init(&badgePK);
  mbedtls_pk_init(&nodePK);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  
  if ((ret = mbedtls_pk_parse_keyfile(&badgePK, "/spiffs/private.pem", "")) != 0) {
    Serial.print(F("ERROR: Failed to load badge private key, error code: "));
    Serial.println(ret);
    teardownCrypto();
    return false;
  }
  
  if ((ret = mbedtls_pk_parse_public_keyfile(&nodePK, "/spiffs/cactuscoinapi.pem") ) != 0) {
    Serial.print(F("ERROR: Failed to load cactuscoinapi public key, error code: "));
    Serial.println(ret);
    teardownCrypto();
    return false;
  }
  
  if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
  {
    Serial.print(F("ERROR: Failed to seed the random number generator , error code: "));
    Serial.println(ret);
    teardownCrypto();
    return false;
  }

  return true;
}

void teardownCrypto() {
  mbedtls_pk_free( &badgePK );
  mbedtls_pk_free( &nodePK );
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );
}

int sign(unsigned char *toSign, int toSignLen, unsigned char *buf, size_t *olen) {
  unsigned char hash[32];
  int ret;
  
  mbedtls_sha256_ret(toSign, toSignLen, hash, 0);
  //Serial.println((char *)toSign);
  //Serial.println(toSignLen);
  /*printf("Here is the sha256:\n");
  for (int i = 0; i < 32; i++)
  {
      printf("%02X", hash[i]);
  }
  printf("\n");*/
  
  if( ( ret = mbedtls_pk_sign( &badgePK, MBEDTLS_MD_SHA256, hash, 0, buf, olen,
                         mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
  {
    Serial.print(F("ERROR: Failed to sign, error code: "));
    Serial.println(ret);
    return -1;
  }
}

bool verify(unsigned char *toVerify, int toVerifyLen, unsigned char *sig, size_t sigLen) {
  unsigned char hash[32];
  int ret;
  Serial.println((char *)toVerify);
  Serial.printf("toVerifyLen: %d sigLen: %d\r\r\n", toVerifyLen, sigLen);
  mbedtls_sha256_ret(toVerify, toVerifyLen, hash, 0);
  
  if( ( ret = mbedtls_pk_verify( &nodePK, MBEDTLS_MD_SHA256, hash, 0, sig, sigLen ) ) != 0 )
  {
    Serial.print(F("ERROR: Failed to verify, error code: "));
    Serial.printf("-0x%04x\r\r\n", -ret);
    return false;
  }

  return true;
}
