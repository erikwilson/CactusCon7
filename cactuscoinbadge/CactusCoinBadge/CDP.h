typedef struct GlobalBroadcast {
	  uint16_t badgeID;  
};

typedef struct Coin {
	  uint16_t CSRID;
    uint16_t broadcasterID;
};

typedef struct CoinSigningRequest {
	  Coin coin;
    byte signatureCSR[CDP_MODULUS_SIZE];
};

typedef struct SignedCoin {
	  CoinSigningRequest csr;
    byte signatureBroadcaster[CDP_MODULUS_SIZE];
};
