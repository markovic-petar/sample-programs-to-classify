cmake_minimum_required(VERSION 3.23)
project(SamplePrograms LANGUAGES C)

# Detect architecture
message(STATUS "Detected processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")

# Source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library directories
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")

    message(STATUS "Configuring for host (x86_64)...")
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openssl/out/x64")
    set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib/out/x64/install")
    set(FFTW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/fftw/out/x64/install")

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")

    message(STATUS "Configuring for ARM Cortex-A8...")
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openssl/out/arm-static")
    set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib/out/arm-static/install")
    set(FFTW3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/fftw/out/arm-static/install")

else()

    message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}. "
                        "Use -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake for ARM, "
                        "or build natively on x86_64.")

endif()

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(FFTW3 REQUIRED)

#########################################################
###   EXECUTABLES                                     ###
#########################################################

function(add_sample_target name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS" ${ARGN})

    if(NOT ARG_SOURCES)
        set(ARG_SOURCES ${SOURCE_DIR}/${name}.c)
    endif()

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE helper ${ARG_LIBS})
endfunction()

# --- CONFIG (header-only) ---

add_library(config INTERFACE)
target_include_directories(config INTERFACE ${INCLUDE_DIR})

# --- CRYPTO_DATA (header-only) ---

add_library(crypto_data INTERFACE)
target_include_directories(crypto_data INTERFACE ${INCLUDE_DIR}/openssl_examples)

# --- HELPER ---

add_library(helper STATIC
    ${SOURCE_DIR}/helper/helper_io.c
    ${SOURCE_DIR}/helper/helper_rng.c
)

target_include_directories(helper PUBLIC ${INCLUDE_DIR}/helper)
target_link_libraries(helper PRIVATE config)

# --- HELPER_TEST ---

add_sample_target(helper_test SOURCES ${SOURCE_DIR}/helper/helper_test.c)

# --- IDLE ---

add_sample_target(idle LIBS config)

#########################################################
###   LINALG                                          ###
#########################################################

# --- VECTOR_ADD ---

add_sample_target(vector_add)

# --- VECTOR_ADD_FP ---

add_sample_target(vector_add_fp)

# --- VECTOR_MUL ---

add_sample_target(vector_mul)

# --- VECTOR_MUL_FP ---

add_sample_target(vector_mul_fp)

# --- MATRIX_MUL ---

add_sample_target(matrix_mul)

# --- MATRIX_MUL_FP ---

add_sample_target(matrix_mul_fp)

#########################################################
###   CRYPTO                                          ###
#########################################################

# --- RSA_ENCRYPT ---

add_sample_target(rsa_encrypt SOURCES ${SOURCE_DIR}/openssl_examples/rsa_encrypt.c LIBS OpenSSL::Crypto crypto_data)

# --- AESGCM ---

add_sample_target(aesgmc SOURCES ${SOURCE_DIR}/openssl_examples/aesgmc.c LIBS OpenSSL::Crypto crypto_data)

# --- CRYPTO_RSA_ENCRYPT ---

add_sample_target(crypto_rsa_encrypt SOURCES ${SOURCE_DIR}/crypto_rsa_encrypt.c LIBS OpenSSL::Crypto crypto_data)

# --- CRYPTO_RSA_DECRYPT ---

add_sample_target(crypto_rsa_decrypt SOURCES ${SOURCE_DIR}/crypto_rsa_decrypt.c LIBS OpenSSL::Crypto crypto_data)

# --- CRYPTO_AES_ENCRYPT ---

add_sample_target(crypto_aes_encrypt SOURCES ${SOURCE_DIR}/crypto_aes_encrypt.c LIBS OpenSSL::Crypto crypto_data)

# --- CRYPTO_AES_DECRYPT ---

add_sample_target(crypto_aes_decrypt SOURCES ${SOURCE_DIR}/crypto_aes_decrypt.c LIBS OpenSSL::Crypto crypto_data)

# --- CRYPTO_RSA_KEYGEN ---

add_sample_target(crypto_rsa_keygen SOURCES ${SOURCE_DIR}/crypto_rsa_keygen.c LIBS OpenSSL::Crypto)

# --- CRYPTO_EC_KEYGEN ---

add_sample_target(crypto_ec_keygen SOURCES ${SOURCE_DIR}/crypto_ec_keygen.c LIBS OpenSSL::Crypto)

#########################################################
###   GZIP                                            ###
#########################################################

# --- GZIP_DEFLATE ---

add_sample_target(gzip_deflate SOURCES ${SOURCE_DIR}/gzip_deflate.c LIBS ZLIB::ZLIB config)

# --- GZIP_INFLATE ---

add_sample_target(gzip_inflate SOURCES ${SOURCE_DIR}/gzip_inflate.c LIBS ZLIB::ZLIB config)

#########################################################
###   SORT                                            ###
#########################################################

# --- SORT_QUICK ---

add_sample_target(sort_quick)

# --- SORT_SELECTION ---

add_sample_target(sort_selection)

#########################################################
###   MISC                                            ###
#########################################################

# --- FFT ---

add_sample_target(fft LIBS FFTW3::fftw3 m)

# --- PTR_CHASE ---

add_sample_target(ptr_chase)

