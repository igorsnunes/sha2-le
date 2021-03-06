// Define this macro when compiling (See Makefile):
// SHA_BITS      256 or 512

// To compile with libcrypto, define LIBCRYPTO

#if (SHA_BITS != 256 && SHA_BITS != 512)
 #error "SHA_BITS should be 256 or 512"
#endif // SHA_BITS

#ifdef LIBCRYPTO
 #include <openssl/evp.h>
 #include <openssl/sha.h>

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
#define OPENSSL_VERSION_1_1 1
#endif

#ifndef OPENSSL_VERSION_1_1
/* For some reason, this function is not declared on OpenSSL's headers */
void OPENSSL_cpuid_setup(void);
#endif

#else
 #include "sha2_common.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

int main (int argc, char *argv[]) {
  char *filename = argv[1];
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    fprintf(stderr, "Error opening %s. %s\n", argv[1],
    strerror(errno));
    return errno;
  }

  // Get file size
  struct stat st;

  if (stat(argv[1], &st) != 0) {
    fprintf(stderr, "Cannot determine size of %s: %s\n", argv[1],
    strerror(errno));
    fclose(file);
    return errno;
  }

  size_t input_size;
#ifdef LIBCRYPTO
  input_size = st.st_size;
#else
  // Padding. input_size is total message bytes including pad bytes.
  input_size = calculate_padded_msg_size(st.st_size);
#endif

  // Load file into an input buffer.
  unsigned char* input = (unsigned char *) calloc(input_size,
                                                  sizeof(unsigned char));
  if (input == NULL) {
    fprintf(stderr, "%s\n.", strerror(errno));
    return errno;
  }

  if (fread(input, sizeof(unsigned char), st.st_size, file) != st.st_size) {
    fprintf(stderr, "Read error on file %s. %s\n", argv[1],
    strerror(errno));
    fclose(file);
    return errno;
  }
  fclose(file);

#ifdef LIBCRYPTO
#ifndef OPENSSL_VERSION_1_1
  /* This needs to be called in order to use a SIMD approach for hashing */
  OPENSSL_cpuid_setup();
#endif

  unsigned char md[SHA_BITS/8];

#if SHA_BITS == 256
 #define SHA SHA256
#elif SHA_BITS == 512
 #define SHA SHA512
#endif

  SHA(input, st.st_size, md);

#undef SHA

  for (int i = 0; i < SHA_BITS/8; i++) {
    printf("%02x", md[i]);
  }
  printf("\n");

  return 0;

#else // LIBCRYPTO not defined
  return sha2(input, st.st_size, input_size);
#endif
}
