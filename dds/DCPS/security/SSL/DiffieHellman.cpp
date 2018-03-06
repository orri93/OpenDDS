/*
 * Distributed under the DDS License.
 * See: http://www.DDS.org/license.html
 */

#include "DiffieHellman.h"
#include "Utils.h"
#include <openssl/dh.h>
#include <cstring>
#include <cerrno>

namespace OpenDDS {
  namespace Security {
    namespace SSL {

      DiffieHellman::DiffieHellman() : k_(NULL)
      {
        EVP_PKEY* pkey = EVP_PKEY_new();
        if (pkey) {
          if (1 == EVP_PKEY_set1_DH(pkey, DH_get_2048_256())) {

            EVP_PKEY_CTX* keygen_ctx = EVP_PKEY_CTX_new(pkey, NULL);
            if (keygen_ctx) {
              if (1 == EVP_PKEY_keygen_init(keygen_ctx)) {
                if (1 == EVP_PKEY_keygen(keygen_ctx, &pkey)) {
                  k_ = pkey;

                } else {
                  fprintf(stderr, "DiffieHellman::load: Error, EVP_PKEY_keygen failed\n");
                }

              } else {
                fprintf(stderr, "DiffieHellman::load: Error, EVP_PKEY_keygen_init failed\n");
              }

            } else {
              fprintf(stderr, "DiffieHellman::load: Error, EVP_PKEY_CTX_new allocation failed\n");
            }

            EVP_PKEY_CTX_free(keygen_ctx);

          } else {
            fprintf(stderr, "DiffieHellman::load: Error, failed to set EVP_PKEY to DH_get_2048_256()\n");
          }

        } else {
          fprintf(stderr, "DiffieHellman::load: Error, failed to allocate new EVP_PKKEY\n");
        }
      }

      DiffieHellman::~DiffieHellman()
      {
        if (k_) EVP_PKEY_free(k_);
      }

      DiffieHellman& DiffieHellman::operator=(const DiffieHellman& rhs)
      {
        if (this != &rhs) {
          if (rhs.k_) {
            k_ = rhs.k_;
            EVP_PKEY_up_ref(k_);

          } else {
            k_ = NULL;
          }
        }
        return *this;
      }

      int DiffieHellman::pub_key(DDS::OctetSeq& dst)
      {
        int result = 1;

        if (k_) {
          DH* dh = EVP_PKEY_get1_DH(k_);
          if (dh) {

            const BIGNUM *pubkey, *privkey; /* Ignore the privkey but pass it in anyway since nothing documents what happens when a NULL gets passed in */
            pubkey = privkey = NULL;
            DH_get0_key(dh, &pubkey, &privkey);
            if (pubkey) {

              dst.length(BN_num_bytes(pubkey));
              if (0 < BN_bn2bin(pubkey, dst.get_buffer())) {
                  result = 0;

              } else {
                  fprintf(stderr, "DiffieHellman::pub_key: Error, BN_bn2bin failed\n");
              }
            }
          }
        }
        return result;
      }

    }
  }
}