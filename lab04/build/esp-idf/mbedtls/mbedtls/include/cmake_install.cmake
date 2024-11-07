# Install script for directory: /opt/esp5/esp-idf/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/opt/esp5/tools/xtensa-esp-elf/esp-13.2.0_20230928/xtensa-esp-elf/bin/xtensa-esp32-elf-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/build_info.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/compat-2.x.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_adjust_legacy_crypto.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_adjust_legacy_from_psa.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_adjust_psa_from_legacy.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_adjust_psa_superset_legacy.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_adjust_ssl.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_adjust_x509.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_psa.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/constant_time.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/lms.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/mbedtls_config.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs7.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/private_access.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/psa_util.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha3.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/build_info.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_adjust_auto_enabled.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_adjust_config_key_pair_types.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_adjust_config_synonyms.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_builtin_composites.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_builtin_key_derivation.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_builtin_primitives.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_compat.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_config.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_common.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_composites.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_key_derivation.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_primitives.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_extra.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_legacy.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_platform.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_se_driver.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_sizes.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_struct.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_types.h"
    "/opt/esp5/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_values.h"
    )
endif()

