#############################################################################
##    Kwave                - CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
##                                                                          #
##    This program is free software; you can redistribute it and/or modify  #
##    it under the terms of the GNU General Public License as published by  #
##    the Free Software Foundation; either version 2 of the License, or     #
##    (at your option) any later version.                                   #
##                                                                          #
#############################################################################

#############################################################################
### project name                                                          ###

PROJECT(kwave)

SET(KWAVE_LINGUAS
    en:English
    de:Deutsch
    fr:Francais
    # <- add new languages here...
)

#############################################################################
### build options:                                                        ###

# OPTION(WITH_ALSA "enable playback/recording via ALSA [default=on]" ON)
# OPTION(WITH_BUILTIN_LIBAUDIOFILE "use builtin libaudiofile [default=off]")
# OPTION(WITH_DOC "build online documentation [default=on]" ON)
# OPTION(WITH_FLAC "enable support for FLAC files [default=on]" ON)
# OPTION(WITH_MP3 "enable support for mp3 files [default=on]" ON)
# OPTION(WITH_OGG "enable support for ogg files [default=on]" ON)
# OPTION(WITH_OSS "enable playback/recording via OSS [default=on]" ON)
# OPTION(WITH_ARTS "use aRts for internal streaming [defalt=on]" ON)

#############################################################################
### toplevel build targets:                                               ###

# all            - default target, build all files
# clean          - clean up the current build directory
# doc            - generate docbook files for online help
# html_doc       - generate HTML help (for the web)
# install        - install the package, with support for DESTDIR
# rpm            - create a RPM package + a src.rpm file
# src_rpm        - create the source RPM only
# tarball        - create a tar.bz2 archive with the sources + specfile
# uninstall      - uninstall the package, with support for DESTDIR
# version-labels - update all version numbers and build dates
# tarball        - create a tar.gz file with the sources

#############################################################################
### check for the proper cmake version                                    ###

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4 FATAL_ERROR)
# set(CMAKE_SKIP_BUILD_RPATH OFF)

#############################################################################
### path to additional cmake modules and includes                         ###

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#############################################################################
### Kwave version number                                                  ###

INCLUDE(FindKwaveVersion)

#############################################################################
### check for the CPU we build for                                        ###

EXECUTE_PROCESS(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE MACHINE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "Building for target ${MACHINE}")

STRING(REGEX MATCH "(i.86-*)|(athlon-*)|(pentium-*)" _mach_x86 ${MACHINE})
IF (_mach_x86)
    MESSAGE(STATUS "Found target optimized memcpy() for X86 (from xine)")
    SET(ARCH_X86 1)
ENDIF (_mach_x86)

STRING(REGEX MATCH "(x86_64-*)|(X86_64-*)|(AMD64-*)|(amd64-*)" _mach_x86_64 ${MACHINE})
IF (_mach_x86_64)
    MESSAGE(STATUS "Found target optimized memcpy() for X86_64 (from xine)")
    SET(ARCH_X86_64 1)
ENDIF (_mach_x86_64)

STRING(REGEX MATCH "(ppc-*)|(powerpc-*)" _mach_ppc ${MACHINE})
IF (_mach_ppc)
    MESSAGE(STATUS "Found target optimized memcpy() for PPC (from xine)")
    SET(ARCH_PPC 1)
ENDIF (_mach_ppc)

#############################################################################
### cmake includes                                                        ###

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckTypeSize)
INCLUDE(TestBigEndian)

#############################################################################

SET(CMAKE_VERBOSE_MAKEFILE OFF)

#############################################################################
### toplevel compiler flags                                               ###

CHECK_C_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_C_FLAGS}" C_HAVE_PIC)
CHECK_CXX_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_C_FLAGS}" CXX_HAVE_PIC)
IF (CXX_HAVE_PIC AND C_HAVE_PIC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_SHARED_LIBRARY_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_SHARED_LIBRARY_C_FLAGS}")
ELSE (CXX_HAVE_PIC AND C_HAVE_PIC)
    MESSAGE(FATAL_ERROR "shared library support is missing")
ENDIF (CXX_HAVE_PIC AND C_HAVE_PIC)

#############################################################################

LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/mt)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libgui)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libkwave)

#############################################################################
### KDE 3 support                                                         ###

INCLUDE(KwaveKDESupport)

INCLUDE_DIRECTORIES(
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################################
### checks for big/little endian                                          ###

TEST_BIG_ENDIAN(ENDIANESS_BIG)
IF (NOT ENDIANESS_BIG)
SET(ENDIANESS_LITTLE 1)
ENDIF(NOT ENDIANESS_BIG)

#############################################################################
### checks for needed header files                                        ###

CHECK_INCLUDE_FILES(sys/times.h HAVE_SYS_TIMES_H)
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
SET(_inc dlfcn.h assert.h ctype.h endian.h errno.h
    fcntl.h features.h float.h limits.h sched.h stdarg.h stddef.h
    stdio.h stdlib.h string.h sys/ioctl.h sys/param.h limits.h
    sys/time.h sys/types.h time.h pthread.h)
CHECK_INCLUDE_FILES("${_inc}" REQUIRED_STD_HEADERS)
IF (NOT REQUIRED_STD_HEADERS)
    MESSAGE(FATAL_ERROR "
        unable to find one or more of the following header files:
        ${_inc}")
ENDIF (NOT REQUIRED_STD_HEADERS)

CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(stddef.h HAVE_STDDEF_H)

#############################################################################
### checks for some data types                                            ###

CHECK_TYPE_SIZE("size_t" SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("long" SIZEOF_LONG)

#############################################################################
### checks for some functions                                             ###

SET(_try_src_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
SET(_try_meminfo "${_try_src_dir}/cmake_try_meminfo.c")
WRITE_FILE("${_try_meminfo}" "
    #include <sys/sysinfo.h>
    #include <linux/kernel.h>
    int main()
    {
        struct sysinfo info;
        unsigned int total;
        sysinfo(&info);
        total = info.totalram;
        return 0;
    }
")
TRY_COMPILE(HAVE_MEMINFO ${CMAKE_BINARY_DIR} ${_try_meminfo} OUTPUT_VARIABLE _out)
IF (NOT HAVE_MEMINFO)
    MESSAGE(STATUS "unable to get memory information through 'sysinfo'")
    MESSAGE(STATUS
        "!!! You might get inaccurate behaviour of the memory management !!!")
ENDIF(NOT HAVE_MEMINFO)

CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(unlink HAVE_UNLINK)

#############################################################################
### optionally: builtin libaudiofile                                      ###

INCLUDE(KwaveLibaudiofileSupport)

#############################################################################
### optionally: OSS and ALSA support for playback and recording           ###

INCLUDE(KwaveOSSSupport)
INCLUDE(KwaveALSASupport)

#############################################################################
### cmake includes                                                        ###

FIND_PACKAGE(RequiredProgram REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)
INCLUDE(FindARTS)
INCLUDE(MCOP)

#############################################################################
### subdirs                                                               ###

ADD_SUBDIRECTORY( mt )
ADD_SUBDIRECTORY( libgui )
ADD_SUBDIRECTORY( libkwave )
ADD_SUBDIRECTORY( kwave )
ADD_SUBDIRECTORY( plugins )
ADD_SUBDIRECTORY( po )

OPTION(WITH_DOC "build online documentation [default=on]" ON)
IF (WITH_DOC)
    ADD_SUBDIRECTORY( doc )
ENDIF (WITH_DOC)

#############################################################################
### target package-messages                                               ###

INCLUDE(KwavePackageMessages)

#############################################################################
### RPM support                                                           ###

INCLUDE(KwaveRPMSupport)

#############################################################################

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

#############################################################################
### Gentoo support                                                        ###

INCLUDE(KwaveEbuild)

#############################################################################
### install files                                                         ###

INSTALL(
    FILES AUTHORS TODO CHANGES README
    DESTINATION ${KDE3_DOCDIR}/kwave-${KWAVE_VERSION_FULL}
)

#############################################################################
### uninstall support                                                     ###

FIND_REQUIRED_PROGRAM(RMDIR_EXECUTABLE rmdir)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY
)

ADD_CUSTOM_TARGET(uninstall
    EXTRA_UNINSTALL_FILES="$ENV{EXTRA_UNINSTALL_FILES}"
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

#############################################################################
### "make version-labels"                                                 ###

ADD_CUSTOM_TARGET(version-labels
    ${CMAKE_SOURCE_DIR}/bin/set_version.sh
        ${CMAKE_SOURCE_DIR}
        ${KWAVE_VERSION_FULL}
        ${KWAVE_VERSION_DATE_ISO}
)

MESSAGE(STATUS "Using CFLAGS=${CMAKE_C_FLAGS}")
MESSAGE(STATUS "Using CXXFLAGS=${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "Using LDFLAGS=${LDFLAGS}")

#original Makefile.am contents follow:

#apidoc:
#	-rm -Rf `find . -name \*.moc`
#	doxygen doxy.cfg

#############################################################################
#############################################################################