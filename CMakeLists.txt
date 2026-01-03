cmake_minimum_required(VERSION 3.23)
project(SamplePrograms LANGUAGES C)

# Detect architecture
message(STATUS "Detected processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")

# Common include directories
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/openssl_examples
)

# Source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Library directories
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")

    message(STATUS "Configuring for host (x86_64)...")
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openssl/out/x64")
    set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib/out/x64/install")
    set(FFTW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/fftw-3.3.10/out/x64/install")

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR
       CMAKE_C_COMPILER MATCHES "arm-linux-gnueabihf")

    message(STATUS "Configuring for ARM Cortex-A8...")
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openssl/out/arm-static")
    set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib/out/arm-static/install")
    set(FFTW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/fftw-3.3.10/out/arm-static/install")

else()

    message(WARNING "Unknown processor type, defaulting to host configuration...")
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openssl/out/x64")
    set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib/out/x64/install")
    set(FFTW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/fftw-3.3.10/out/x64/install")

endif()

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(FFTW3 REQUIRED)

#########################################################
###   EXECUTABLES                                     ###
#########################################################

# --- HELPER ---

add_library( helper STATIC
    ${SOURCE_DIR}/helper/helper_io.c
    ${SOURCE_DIR}/helper/helper_rng.c
)

target_include_directories(helper PRIVATE ${INCLUDE_DIRS})

# --- HELPER_TEST ---

add_executable( helper_test
    ${SOURCE_DIR}/helper/helper_test.c
)

target_include_directories(helper_test PRIVATE ${INCLUDE_DIRS})
target_link_libraries(helper_test PRIVATE helper)

# --- IDLE ---

add_executable( idle
    ${SOURCE_DIR}/idle.c
)

#########################################################
###   LINALG                                          ###
#########################################################

target_include_directories(idle PRIVATE ${INCLUDE_DIRS})
target_link_libraries(idle PRIVATE helper)

# --- VECTOR_ADD ---

add_executable( vector_add
    ${SOURCE_DIR}/vector_add.c
)

target_include_directories(vector_add PRIVATE ${INCLUDE_DIRS})
target_link_libraries(vector_add PRIVATE helper)

# --- VECTOR_ADD_FP ---

add_executable( vector_add_fp
    ${SOURCE_DIR}/vector_add_fp.c
)

target_include_directories(vector_add_fp PRIVATE ${INCLUDE_DIRS})
target_link_libraries(vector_add_fp PRIVATE helper)

# --- VECTOR_MUL ---

add_executable( vector_mul
    ${SOURCE_DIR}/vector_mul.c
)

target_include_directories(vector_mul PRIVATE ${INCLUDE_DIRS})
target_link_libraries(vector_mul PRIVATE helper)

# --- VECTOR_MUL_FP ---

add_executable( vector_mul_fp
    ${SOURCE_DIR}/vector_mul_fp.c
)

target_include_directories(vector_mul_fp PRIVATE ${INCLUDE_DIRS})
target_link_libraries(vector_mul_fp PRIVATE helper)

# --- MATRIX_MUL ---

add_executable( matrix_mul
    ${SOURCE_DIR}/matrix_mul.c
)

target_include_directories(matrix_mul PRIVATE ${INCLUDE_DIRS})
target_link_libraries(matrix_mul PRIVATE helper)

# --- MATRIX_MUL_FP ---

add_executable( matrix_mul_fp
    ${SOURCE_DIR}/matrix_mul_fp.c
)

target_include_directories(matrix_mul_fp PRIVATE ${INCLUDE_DIRS})
target_link_libraries(matrix_mul_fp PRIVATE helper)

#########################################################
###   CRYPTO                                          ###
#########################################################

# --- RSA_ENCRYPT ---

add_executable( rsa_encrypt
    ${SOURCE_DIR}/openssl_examples/rsa_encrypt.c
)

target_include_directories(rsa_encrypt PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(rsa_encrypt PRIVATE helper OpenSSL::Crypto)

# --- AESGCM ---

add_executable( aesgmc
    ${SOURCE_DIR}/openssl_examples/aesgmc.c
)

target_include_directories(aesgmc PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(aesgmc PRIVATE helper OpenSSL::Crypto)

# --- CRYPTO_RSA_ENCRYPT ---

add_executable( crypto_rsa_encrypt
    ${SOURCE_DIR}/crypto_rsa_encrypt.c
)

target_include_directories(crypto_rsa_encrypt PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(crypto_rsa_encrypt PRIVATE helper OpenSSL::Crypto)

# --- CRYPTO_RSA_DECRYPT ---

add_executable( crypto_rsa_decrypt
    ${SOURCE_DIR}/crypto_rsa_decrypt.c
)

target_include_directories(crypto_rsa_decrypt PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(crypto_rsa_decrypt PRIVATE helper OpenSSL::Crypto)

# --- CRYPTO_AES_ENCRYPT ---

add_executable( crypto_aes_encrypt
    ${SOURCE_DIR}/crypto_aes_encrypt.c
)

target_include_directories(crypto_aes_encrypt PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(crypto_aes_encrypt PRIVATE helper OpenSSL::Crypto)

# --- CRYPTO_AES_DECRYPT ---

add_executable( crypto_aes_decrypt
    ${SOURCE_DIR}/crypto_aes_decrypt.c
)

target_include_directories(crypto_aes_decrypt PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(crypto_aes_decrypt PRIVATE helper OpenSSL::Crypto)

# --- CRYPTO_RSA_KEYGEN ---

add_executable( crypto_rsa_keygen
    ${SOURCE_DIR}/crypto_rsa_keygen.c
)

target_include_directories(crypto_rsa_keygen PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(crypto_rsa_keygen PRIVATE helper OpenSSL::Crypto)

# --- CRYPTO_EC_KEYGEN ---

add_executable( crypto_ec_keygen
    ${SOURCE_DIR}/crypto_ec_keygen.c
)

target_include_directories(crypto_ec_keygen PRIVATE ${INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(crypto_ec_keygen PRIVATE helper OpenSSL::Crypto)

#########################################################
###   GZIP                                            ###
#########################################################

# --- GZIP_DEFLATE ---

add_executable( gzip_deflate
    ${SOURCE_DIR}/gzip_deflate.c
)

target_include_directories(gzip_deflate PRIVATE ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(gzip_deflate PRIVATE helper ${ZLIB_LIBRARIES})

# --- GZIP_INFLATE ---

add_executable( gzip_inflate
    ${SOURCE_DIR}/gzip_inflate.c
)

target_include_directories(gzip_inflate PRIVATE ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(gzip_inflate PRIVATE helper ${ZLIB_LIBRARIES})

#########################################################
###   SORT                                            ###
#########################################################

# --- SORT_QUICK ---

add_executable( sort_quick
    ${SOURCE_DIR}/sort_quick.c
)

target_include_directories(sort_quick PRIVATE ${INCLUDE_DIRS})
target_link_libraries(sort_quick PRIVATE helper)

# --- SORT_SELECTION ---

add_executable( sort_selection
    ${SOURCE_DIR}/sort_selection.c
)

target_include_directories(sort_selection PRIVATE ${INCLUDE_DIRS})
target_link_libraries(sort_selection PRIVATE helper)

#########################################################
###   MISC                                            ###
#########################################################

# --- FFT ---

add_executable( fft
    ${SOURCE_DIR}/fft.c
)

target_include_directories(fft PRIVATE ${INCLUDE_DIRS} ${FFTW3_INCLUDE_DIRS})
target_link_libraries(fft PRIVATE helper FFTW3::fftw3 m)

# --- PTR_CHASE ---

add_executable( ptr_chase
    ${SOURCE_DIR}/ptr_chase.c
)

target_include_directories(ptr_chase PRIVATE ${INCLUDE_DIRS})
target_link_libraries(ptr_chase PRIVATE helper)

