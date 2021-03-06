cmake_minimum_required(VERSION 3.13)
project(wowlet)

message(STATUS "Initiating compile using CMake ${CMAKE_VERSION}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(VERSION_MAJOR "3")
set(VERSION_MINOR "0")
set(VERSION_REVISION "0")
set(VERSION "beta-4")

option(FETCH_DEPS "Download dependencies if they are not found" ON)
option(OPENVR "Include OpenVR support")
option(ANDROID "Android deployment")
option(ANDROID_DEBUG "View the Android app on desktop")
option(TOR_BIN "Path to Tor binary to embed inside WOWlet")
option(STATIC "Link libraries statically, requires static Qt")
option(USE_DEVICE_TREZOR "Trezor support compilation")
option(DONATE_BEG "Prompt donation window every once in a while" ON)
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)
include(FetchContent)
include(FindCcache)
include(CheckIncludeFile)
include(CheckSymbolExists)

set(WOWNERO_HEAD "f611d5c9e32bc62f1735f6571b0bdb95cc020531")
set(BUILD_GUI_DEPS ON)
set(BUILD_64 ON CACHE BOOL "Build 64-bit binaries")
set(INSTALL_VENDORED_LIBUNBOUND ${STATIC})
set(USE_SINGLE_BUILDDIR ON)

# Are we in debug mode?
set(_CMAKE_BUILD_TYPE "")
string(TOUPPER "${CMAKE_BUILD_TYPE}" _CMAKE_BUILD_TYPE)
if("${_CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(DEBUG ON)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

check_include_file(sys/prctl.h HAVE_SYS_PRCTL_H)
check_symbol_exists(prctl "sys/prctl.h" HAVE_PRCTL)

if(STATIC)
    message(STATUS "Initiating static build, turning on manual submodules")
    set(MANUAL_SUBMODULES 1)

    # monero-project/unbound:monero has a fix for static builds, however, it's not merged in Monero yet, because
    # it breaks their buildbot, since that still uses openssl 1.1.0 and we use openssl 1.1.1g. We need to
    # manually set the unbound submodule the right commit that has the fix.
    # This only works with -DMANUAL_SUBMODULES=1
    message(STATUS "applying unbound static build fix contrib/unbound_static.patch")
    execute_process(COMMAND bash -c "git -C ${CMAKE_SOURCE_DIR}/wownero/external/unbound apply ${CMAKE_SOURCE_DIR}/contrib/unbound_static.patch")

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    add_definitions(-DMONERO_GUI_STATIC)
endif()

function (add_c_flag_if_supported flag var)
    string(REPLACE "-" "_" supported ${flag}_c)
    check_c_compiler_flag(${flag} ${supported})
    if(${${supported}})
        set(${var} "${${var}} ${flag}" PARENT_SCOPE)
    endif()
endfunction()

function (add_cxx_flag_if_supported flag var)
    string(REPLACE "-" "_" supported ${flag}_cxx)
    check_cxx_compiler_flag(${flag} ${supported})
    if(${${supported}})
        set(${var} "${${var}} ${flag}" PARENT_SCOPE)
    endif()
endfunction()

function (add_linker_flag_if_supported flag var)
    string(REPLACE "-" "_" supported ${flag}_ld)
    string(REPLACE "," "_" supported ${flag}_ld)
    check_linker_flag(${flag} ${supported})
    if(${${supported}})
        set(${var} "${${var}} ${flag}" PARENT_SCOPE)
    endif()
endfunction()

find_package(Git)
#if(GIT_FOUND)
#    message(STATUS "Initializing submodules")
#    execute_process(COMMAND git "submodule" "update" "--init" "--recursive")
#    execute_process(COMMAND git rev-parse "HEAD" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/wownero OUTPUT_VARIABLE _WOWNERO_HEAD OUTPUT_STRIP_TRAILING_WHITESPACE)
#    if(NOT _WOWNERO_HEAD STREQUAL WOWNERO_HEAD)
#        message(FATAL_ERROR "[submodule] Wownero HEAD was at ${_WOWNERO_HEAD} but should be at ${WOWNERO_HEAD}")
#    else()
#        message(STATUS "[submodule] Wownero HEAD @ ${WOWNERO_HEAD}")
#    endif()
#endif()

add_subdirectory(wownero)
get_directory_property(ARCH_WIDTH DIRECTORY "wownero" DEFINITION ARCH_WIDTH)

include(CMakePackageConfigHelpers)
include(VersionMonero)
include(VersionWowlet)

include_directories(${EASYLOGGING_INCLUDE})
link_directories(${EASYLOGGING_LIBRARY_DIRS})

# OpenSSL
if(APPLE AND NOT OPENSSL_ROOT_DIR)
    execute_process(COMMAND brew --prefix openssl OUTPUT_VARIABLE OPENSSL_ROOT_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL: Version ${OPENSSL_VERSION}")
message(STATUS "OpenSSL: include dir at ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL: libraries at ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES}")

# Sodium
find_library(SODIUM_LIBRARY sodium)
message(STATUS "libsodium: libraries at ${SODIUM_LIBRARY}")

# HIDApi
set(HIDAPI_FOUND OFF)

# QrEncode
find_package(QREncode REQUIRED)

# Tevador 14 word Monero seed
find_package(monero-seed CONFIG)
if(NOT monero-seed_FOUND)
  if(FETCH_DEPS)
    FetchContent_Declare(monero-seed
      GIT_REPOSITORY https://git.wownero.com/wowlet/monero-seed.git)
    FetchContent_GetProperties(monero-seed)
    if(NOT monero-seed_POPULATED)
      message(STATUS "Fetching monero-seed")
      FetchContent_Populate(monero-seed)
      add_subdirectory(${monero-seed_SOURCE_DIR} ${monero-seed_BINARY_DIR})
    endif()
    add_library(monero-seed::monero-seed ALIAS monero-seed)
  else()
    message(FATAL_ERROR "monero-seed was not installed and fetching deps is disabled")
  endif()
endif()

# Boost
if(DEBUG)
    set(Boost_DEBUG ON)
endif()
if(APPLE AND NOT BOOST_ROOT)
    execute_process(COMMAND brew --prefix boost OUTPUT_VARIABLE BOOST_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(MINGW)
    set(Boost_THREADAPI win32)
endif()
find_package(Boost 1.58 REQUIRED COMPONENTS
        system
        filesystem
        thread
        date_time
        chrono
        regex
        serialization
        program_options
        locale)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        # https://github.com/monero-project/monero-gui/issues/3142#issuecomment-705940446
        set(CMAKE_SKIP_RPATH ON)
    endif()

    find_package(X11 REQUIRED)
    message(STATUS "X11_FOUND = ${X11_FOUND}")
    message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
    message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")
    include_directories(${X11_INCLUDE_DIR})
    link_directories(${X11_LIBRARIES})
    if(STATIC)
        find_library(XCB_LIBRARY xcb)
        message(STATUS "Found xcb library: ${XCB_LIBRARY}")
    endif()
endif()

if("$ENV{DRONE}" STREQUAL "true")
    message(STATUS "We are inside a static compile with Drone CI")
endif()

if(UNIX)
    if(NOT CMAKE_PREFIX_PATH AND DEFINED ENV{CMAKE_PREFIX_PATH})
        message(STATUS "Using CMAKE_PREFIX_PATH environment variable: '$ENV{CMAKE_PREFIX_PATH}'")
        set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
    endif()
    if(APPLE AND NOT CMAKE_PREFIX_PATH)
        execute_process(COMMAND brew --prefix qt5 OUTPUT_VARIABLE QT5_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND CMAKE_PREFIX_PATH ${QT5_DIR})
    endif()
endif()

# To build WOWlet with embedded (and static) Tor, pass CMake -DTOR_BIN=/path/to/tor
if(TOR_BIN)
    if(APPLE)
        execute_process(COMMAND bash -c "touch ${CMAKE_CURRENT_SOURCE_DIR}/src/tor/libevent-2.1.7.dylib")
    endif()

    execute_process(COMMAND bash -c "${TOR_BIN} --version --quiet" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE out RESULT_VARIABLE ret)
    if (ret EQUAL "0")
        set(TOR_VERSION "${out}")
    endif()
    message(STATUS "${TOR_VERSION}")
    configure_file("cmake/config-wowlet.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-wowlet.h")

    # on the buildbot Tor is baked into the image
    # - linux: See `Dockerfile`
    # - windows: See `Dockerfile.windows`
    # - macos: taken from Tor Browser official release
    if(REPRODUCIBLE) # Always copy Tor when doing a reproducible build to prevent old versions from getting included
        set(TOR_COPY_CMD "cp ${TOR_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/src/assets/exec/tor")
    else()
        set(TOR_COPY_CMD "cp -u ${TOR_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/src/assets/exec/tor")
    endif()
    message(STATUS "${TOR_COPY_CMD}")
    execute_process(COMMAND bash -c "${TOR_COPY_CMD}" RESULT_VARIABLE ret)
    if(ret EQUAL "1")
        message(FATAL_ERROR "Tor copy failure: ${TOR_COPY_CMD}")
    endif()

    message(STATUS "Embedding Tor binary at ${TOR_BIN}")
else()
    message(STATUS "Skipping Tor inclusion because -DTOR_BIN=Off")
endif()

if(MINGW)
    string(REGEX MATCH "^[^/]:/[^/]*" msys2_install_path "${CMAKE_C_COMPILER}")
    message(STATUS "MSYS location: ${msys2_install_path}")
    set(CMAKE_INCLUDE_PATH "${msys2_install_path}/mingw${ARCH_WIDTH}/include")
    # This is necessary because otherwise CMake will make Boost libraries -lfoo
    # rather than a full path. Unfortunately, this makes the shared libraries get
    # linked due to a bug in CMake which misses putting -static flags around the
    # -lfoo arguments.
    set(DEFLIB ${msys2_install_path}/mingw${ARCH_WIDTH}/lib)
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_DIRECTORIES ${DEFLIB})
    list(REMOVE_ITEM CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES ${DEFLIB})
endif()

message(STATUS "Using Boost include dir at ${Boost_INCLUDE_DIRS}")
message(STATUS "Using Boost libraries at ${Boost_LIBRARIES}")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
if(MINGW)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
    set(EXTRA_LIBRARIES mswsock;ws2_32;iphlpapi;crypt32;bcrypt)
    if(DEPENDS)
	    set(ICU_LIBRARIES iconv)
    else()
        set(ICU_LIBRARIES icuio icuin icuuc icudt icutu iconv)
    endif()
elseif(APPLE)
    set(EXTRA_LIBRARIES "-framework AppKit")
elseif(OPENBSD)
    set(EXTRA_LIBRARIES "")
elseif(FREEBSD)
    set(EXTRA_LIBRARIES execinfo)
elseif(DRAGONFLY)
    find_library(COMPAT compat)
    set(EXTRA_LIBRARIES execinfo ${COMPAT})
elseif(CMAKE_SYSTEM_NAME MATCHES "(SunOS|Solaris)")
    set(EXTRA_LIBRARIES socket nsl resolv)
elseif(NOT MSVC AND NOT DEPENDS AND NOT ANDROID)
    find_library(RT rt)
    set(EXTRA_LIBRARIES ${RT})
endif()

list(APPEND EXTRA_LIBRARIES ${CMAKE_DL_LIBS})

if(APPLE)
    include_directories(SYSTEM /usr/include/malloc)
    if(POLICY CMP0042)
        cmake_policy(SET CMP0042 NEW)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -fvisibility=default -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -DGTEST_HAS_TR1_TUPLE=0")
endif()

if (APPLE AND NOT IOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -fvisibility=default -std=c++11")
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -DGTEST_HAS_TR1_TUPLE=0")
endif()

# warnings
# @TODO: enable these 2 for migration to Qt 6
#add_c_flag_if_supported(-Werror C_SECURITY_FLAGS)
#add_cxx_flag_if_supported(-Werror CXX_SECURITY_FLAGS)
add_c_flag_if_supported(-Wformat C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-Wformat CXX_SECURITY_FLAGS)
add_c_flag_if_supported(-Wformat-security C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-Wformat-security CXX_SECURITY_FLAGS)

# -fstack-protector
if (NOT OPENBSD AND NOT (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1)))
    add_c_flag_if_supported(-fstack-protector C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-protector CXX_SECURITY_FLAGS)
    add_c_flag_if_supported(-fstack-protector-strong C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-protector-strong CXX_SECURITY_FLAGS)
endif()

# New in GCC 8.2
if (NOT OPENBSD AND NOT (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1)))
    add_c_flag_if_supported(-fcf-protection=full C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fcf-protection=full CXX_SECURITY_FLAGS)
endif()
if (NOT WIN32 AND NOT OPENBSD)
    add_c_flag_if_supported(-fstack-clash-protection C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-fstack-clash-protection CXX_SECURITY_FLAGS)
endif()

# Removed in GCC 9.1 (or before ?), but still accepted, so spams the output
if (NOT (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 9.1))
    add_c_flag_if_supported(-mmitigate-rop C_SECURITY_FLAGS)
    add_cxx_flag_if_supported(-mmitigate-rop CXX_SECURITY_FLAGS)
endif()

# linker
if (APPLE)
    add_linker_flag_if_supported(-Wl,-bind_at_load LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,-dead_strip LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,-dead_strip_dylibs LD_SECURITY_FLAGS)
endif()
if (NOT APPLE AND NOT (WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    # Windows binaries die on startup with PIE when compiled with GCC
    add_linker_flag_if_supported(-pie LD_SECURITY_FLAGS)
endif()
add_linker_flag_if_supported(-Wl,-z,relro LD_SECURITY_FLAGS)
add_linker_flag_if_supported(-Wl,-z,now LD_SECURITY_FLAGS)
add_linker_flag_if_supported(-Wl,-z,noexecstack noexecstack_SUPPORTED)
if (noexecstack_SUPPORTED)
    set(LD_SECURITY_FLAGS "${LD_SECURITY_FLAGS} -Wl,-z,noexecstack")
endif()
add_linker_flag_if_supported(-Wl,-z,noexecheap noexecheap_SUPPORTED)
if (noexecheap_SUPPORTED)
    set(LD_SECURITY_FLAGS "${LD_SECURITY_FLAGS} -Wl,-z,noexecheap")
endif()

# some windows linker bits
if (WIN32)
    add_linker_flag_if_supported(-Wl,--dynamicbase LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,--nxcompat LD_SECURITY_FLAGS)
    add_linker_flag_if_supported(-Wl,--high-entropy-va LD_SECURITY_FLAGS)
endif()

if(STATIC)
    add_linker_flag_if_supported(-static-libgcc STATIC_FLAGS)
    add_linker_flag_if_supported(-static-libstdc++ STATIC_FLAGS)
    if(MINGW)
        add_linker_flag_if_supported(-static STATIC_FLAGS)
    endif()
endif()

# With GCC 6.1.1 the compiled binary malfunctions due to aliasing. Until that
# is fixed in the code (Issue #847), force compiler to be conservative.
add_c_flag_if_supported(-fno-strict-aliasing C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-fno-strict-aliasing CXX_SECURITY_FLAGS)

add_c_flag_if_supported(-fPIC C_SECURITY_FLAGS)
add_cxx_flag_if_supported(-fPIC CXX_SECURITY_FLAGS)

message(STATUS "Using C security hardening flags: ${C_SECURITY_FLAGS}")
message(STATUS "Using C++ security hardening flags: ${CXX_SECURITY_FLAGS}")
message(STATUS "Using linker security hardening flags: ${LD_SECURITY_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ${C_SECURITY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${CXX_SECURITY_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LD_SECURITY_FLAGS} ${STATIC_FLAGS}")

if(APPLE)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/contrib/KDMacTouchBar")
endif()

if(OPENVR)
    # Add contrib/openvr as library
    add_definitions(-DVR_API_PUBLIC)
    add_definitions(-DOPENVR_BUILD_STATIC)  # is this needed?
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/contrib/openvr")
endif()

if(WITH_SCANNER)
    add_library(quirc STATIC
            contrib/quirc/lib/decode.c
            contrib/quirc/lib/identify.c
            contrib/quirc/lib/quirc.c
            contrib/quirc/lib/version_db.c
            )
    target_include_directories(quirc PUBLIC contrib/quirc/lib)
endif()

add_subdirectory(src)
