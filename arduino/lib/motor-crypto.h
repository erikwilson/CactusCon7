#include <Crypto.h>

#define HMAC_KEY_LENGTH 16
#define AES_KEY_LENGTH 16

union MotorMessage {
  uint8_t data[32];
  struct {
    uint8_t random[21];
    uint8_t mode;
    uint8_t motorL;
    uint8_t motorR;
    uint32_t counter;
    uint32_t connectionId;
  };
};

union MotorEncrypted {
  uint8_t data[80];
  uint8_t hmacData[48];
  struct {
    uint8_t iv[16];
    uint8_t message[32];
    uint8_t hmac[32];
  };
};

class MotorCrypto {
  private:

    uint32_t counter = 0;

    union {
      uint32_t connectionId;
      uint8_t  connectionIdBytes[4];
    };

    union KeyHash {
      uint8_t data[32];
      struct {
        uint8_t aes[16];
        uint8_t hmac[16];
      };
    } keyHash;

  public:
    MotorCrypto(
      const uint8_t *key,
      const uint16_t keyLen = 0,
      const uint32_t connId = 0
    ) {
      assert(keyLen != 0);

      SHA256 sha256;
      sha256.doUpdate(key, keyLen);
      sha256.doFinal(keyHash.data);

      if (connId != 0) {
        connectionId = connId;
      } else {
        RNG::fill(connectionIdBytes, sizeof(connectionIdBytes));
      }
    }

    uint32_t getCounter() {
      return counter;
    }

    uint32_t getConnectionId() {
      return connectionId;
    }

    MotorEncrypted encrypt(
      const uint8_t mode,
      const uint8_t motorL,
      const uint8_t motorR
    ) {
      MotorMessage message;
      RNG::fill(message.random, sizeof(message.random));
      message.mode = mode;
      message.motorL = motorL;
      message.motorR = motorR;
      message.counter = ++counter;
      message.connectionId = connectionId;

      MotorEncrypted encrypted;
      RNG::fill(encrypted.iv, sizeof(encrypted.iv));

      assert(sizeof(message.data) == sizeof(encrypted.message));
      assert((sizeof(encrypted.message) % AES_BLOCKSIZE) == 0);

      AES aes(keyHash.aes, encrypted.iv, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
      aes.processNoPad(message.data, encrypted.message, sizeof(message.data));

      SHA256HMAC hmac(keyHash.hmac, HMAC_KEY_LENGTH);
      hmac.doUpdate(encrypted.hmacData, sizeof(encrypted.hmacData));
      hmac.doFinal(encrypted.hmac);

      return encrypted;
    }

    bool verifyHmac(const MotorEncrypted &encrypted) {
      SHA256HMAC hmac(keyHash.hmac, HMAC_KEY_LENGTH);
      hmac.doUpdate(encrypted.hmacData, sizeof(encrypted.hmacData));
      return hmac.matches(encrypted.hmac);
    }

    bool verifyCounter(const MotorMessage &message) {
      return counter < message.counter;
    }

    bool verifyConnectionId(const MotorMessage &message) {
      if (counter == 0) return true;
      return connectionId == message.connectionId;
    }

    bool validateMessage(const MotorMessage &message) {
      if (!verifyCounter(message) || !verifyConnectionId(message)) {
        return false;
      }
      if (counter == 0) {
        connectionId = message.connectionId;
      }
      counter = message.counter;
      return true;
    }

    MotorMessage decrypt(const MotorEncrypted &encrypted) {
      MotorMessage message;
      AES aes(keyHash.aes, encrypted.iv, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
      aes.processNoPad(encrypted.message, message.data, sizeof(message.data));
      return message;
    }
};
