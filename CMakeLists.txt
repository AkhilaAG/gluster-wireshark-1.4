# CMakeLists.txt
#
# $Id$
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 1998 Gerald Combs
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

project(Wireshark C)

cmake_minimum_required(VERSION 2.6)
set(CMAKE_BACKWARDS_COMPATIBILITY 2.6)

# set(PROJECT_VERSION "1.3.4-rc5")
# set(PROJECT_VERSION "1.3.4rc5")
# set(PROJECT_VERSION "1.3.4")
# If not set, copy over Wireshark version from configure.in AC_INIT
if(NOT PROJECT_VERSION)
	file(STRINGS
		${CMAKE_SOURCE_DIR}/configure.in
		VERSIONLINE
		REGEX "\\(wireshark, .*\\)"
	)
	string(REGEX REPLACE ".*\\(wireshark, (.*)\\).*"
		"\\1"
		PROJECT_VERSION
		${VERSIONLINE}
	)
endif()
string(REGEX REPLACE "([0-9]+)\\..*"
	"\\1"
	PROJECT_MAJOR_VERSION
	${PROJECT_VERSION}
)
string(REGEX REPLACE ".*\\.([0-9]+)\\..*"
	"\\1"
	PROJECT_MINOR_VERSION
	${PROJECT_VERSION}
)
string(REGEX REPLACE ".*\\..*\\.([0-9]+).*"
	"\\1"
	PROJECT_PATCH_VERSION
	${PROJECT_VERSION}
)
string(REGEX REPLACE ".*\\..*\\.[0-9]+(.*)"
	"\\1"
	PROJECT_VERSION_EXTENSION
	${PROJECT_VERSION}
)
message(STATUS "V: ${PROJECT_VERSION}, MaV: ${PROJECT_MAJOR_VERSION}, MiV: ${PROJECT_MINOR_VERSION}, PL: ${PROJECT_PATCH_VERSION}, EV: ${PROJECT_VERSION_EXTENSION}.")

#Where to find local cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
INCLUDE(UseLemon)
INCLUDE(UseMakeDissectorReg)
INCLUDE(UseMakeTapReg)

# Under linux the release mode (CMAKE_BUILD_TYPE=release) defines NDEBUG

# Disable this later. Alternative: "make VERBOSE=1"
set(CMAKE_VERBOSE_MAKEFILE ON)

#Defines CMAKE_INSTALL_BINDIR, CMAKE_INSTALL_DATADIR, etc ...
include(CMakeInstallDirs)

include_directories(
	${CMAKE_BINARY_DIR}
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/epan
	${CMAKE_SOURCE_DIR}/tools/lemon
	${CMAKE_SOURCE_DIR}/wiretap
)

#Where to put libraries
if(NOT LIBRARY_OUTPUT_PATH)
	set(LIBRARY_OUTPUT_PATH ${Wireshark_BINARY_DIR}/lib CACHE INTERNAL
		   "Single output directory for building all libraries.")
endif()


include(CMakeOptions.txt)


set(WIRESHARK_C_FLAGS
	-O2
	-Wall
	-W
	-Wextra
	-Wdeclaration-after-statement
	-Wendif-labels
	-Wpointer-arith
	-Wno-pointer-sign
	-Warray-bounds
	-Wcast-align
	-Wformat-security
	-fexcess-precision=fast
)

set(WIRESHARK_EXTRA_C_FLAGS
	-pedantic
	-Woverflow
	-Wlogical-op
	-Wno-overlength-strings
	-fstrict-overflow -Wstrict-overflow=4
	-Wunreachable-code
	-Wunsafe-loop-optimizations
	-Wno-long-long
	-Wbad-function-cast
	-Wcast-qual
	-Waddress
	-Warray-bounds
	-Wattributes
	-Wdiv-by-zero
	-Wformat-security
	-Wignored-qualifiers
	-Wpragmas
	-Wredundant-decls
	-Wvla
	-Wc++-compat
	-Wjump-misses-init
	# packet-ncp2222.inc prevents this from going into all warnings
	-Wwrite-strings
	-Wstrict-prototypes
	-Wmissing-declarations
	# Problem with packet-afs.c
	-Wshadow
	# More cleanup needed for this on LP64
	-Wshorten-64-to-32
)

if(ENABLE_EXTRA_GCC_CHECKS)   # This overrides -Werror
	set(WIRESHARK_C_FLAGS ${WIRESHARK_C_FLAGS} ${WIRESHARK_EXTRA_C_FLAGS})
endif()

add_definitions(
	-DHAVE_CONFIG_H
	-DPACKAGE=\"wireshark\"
	-DNEW_PACKET_LIST
)

# Counterhack to work around some cache magic in CHECK_C_SOURCE_COMPILES
include(CheckCCompilerFlag)
set(C 0)
# Sigh: Have to use THIS_FLAG instead of ${F} for some reason
foreach(THIS_FLAG ${WIRESHARK_C_FLAGS})
	set(F WS_C_FLAG_${C})
	set(${F} ${THIS_FLAG})
	set(V WS_C_FLAG_VALID${C})
	check_c_compiler_flag(${${F}} ${V})
	if (${${V}})
		add_definitions(${${F}})
	endif()
	math(EXPR C "${C} + 1")
endforeach()

if(NOT DISABLE_WERROR AND NOT ENABLE_EXTRA_GCC_CHECKS)
	check_c_compiler_flag(-Werror WERROR)
else()
	set(WERROR 0)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	add_definitions( -D_U_=__attribute__\(\(unused\)\) )
else()
	add_definitions( -D_U_=\"\" )
endif()

set(WIRESHARK_LD_FLAGS
	-Wl,--as-needed
	# -flto
	# -fwhopr
	# -fwhole-program
)

include(CheckCLinkerFlag)
set(C 0)
# Sigh: Have to use THIS_FLAG instead of ${F} for some reason
foreach(THIS_FLAG ${WIRESHARK_LD_FLAGS})
	set(F WS_LD_FLAG_${C})
	set(${F} ${THIS_FLAG})
	set(V WS_LD_FLAG_VALID${C})
	check_c_linker_flag(${${F}} ${V})
	if (${${V}})
		set(WS_LINK_FLAGS ${WS_LINK_FLAGS} ${${F}})
	endif()
	math(EXPR C "${C} + 1")
endforeach()

if(ENABLE_STATIC)
  set(LINK_MODE_LIB STATIC)
  set(LINK_MODE_MODULE STATIC)
else()
  set(LINK_MODE_LIB SHARED)
  set(LINK_MODE_MODULE MODULE)
endif()

# The packagelist is doing some magic:  If we add XXX to the packagelist, we
# - may optionally set XXX_OPTIONS to pass to the find_package command
# - will call FindXXX.cmake
# - return found libraries in XXX_LIBRARIES
# - return found include in XXX_INCLUDE_DIRS
# - set HAVE_XXX

#The minimum package list
set(PACKAGELIST GLIB2 PCAP M LEX YACC Perl SH PythonInterp HtmlViewer ${PACKAGELIST})

#Build the gui ?
if(BUILD_wireshark)
	set(PACKAGELIST GTK2 ${PACKAGELIST})
	set(GTK2_OPTIONS COMPONENTS gtk)
	set(GTK2_DEBUG false)
endif()

# SMI SNMP
if(ENABLE_SMI)
	set(PACKAGELIST SMI ${PACKAGELIST})
endif()

# GNU crypto
if(ENABLE_GCRYPT)
	set(PACKAGELIST GCRYPT ${PACKAGELIST})
endif()

# GNU SSL/TLS support
if(ENABLE_GNUTLS)
	set(PACKAGELIST GNUTLS ${PACKAGELIST})
endif()

# Regular expressions lib
if(ENABLE_PCRE)
	set(PACKAGELIST PCRE ${PACKAGELIST})
endif()

# Kerberos
if(ENABLE_KERBEROS)
	set(PACKAGELIST KERBEROS ${PACKAGELIST})
endif()

# Portable audio
if(ENABLE_PORTAUDIO)
	set(PACKAGELIST PORTAUDIO ${PACKAGELIST})
endif()


# Prefer c-ares over adns
if(ENABLE_CARES)        # C Asynchronouse resolver
	set(PACKAGELIST CARES ${PACKAGELIST})
elseif(ENABLE_ADNS)     # Gnu asynchronous DNS
	set(PACKAGELIST ADNS ${PACKAGELIST})
endif()

# Zlib compression
if(ENABLE_ZLIB)
	set(PACKAGELIST ZLIB ${PACKAGELIST})
endif()

# Lua 5.1 dissectors
if(ENABLE_LUA)
	set(PACKAGELIST LUA ${PACKAGELIST})
endif()

# GeoIP address resolving
if(ENABLE_GEOIP)
	set(PACKAGELIST GEOIP ${PACKAGELIST})
endif()

# Capabilities
if(ENABLE_CAP)
	set(PACKAGELIST CAP ${PACKAGELIST})
endif()

if(ENABLE_PYTHON)
	set(PACKAGELIST PYTHON ${PACKAGELIST})
endif()

set(PROGLIST text2pcap mergecap capinfos editcap dumpcap)

#Let's loop the package list
foreach(PACKAGE ${PACKAGELIST})
	if(${PACKAGE}_OPTIONS)
		find_package(${PACKAGE} ${${PACKAGE}_OPTIONS})
	else()
		find_package(${PACKAGE} REQUIRED)
	endif()
	message(${PACKAGE}_FOUND)
	if (${PACKAGE}_FOUND)
		set(HAVE_LIB${PACKAGE} 1)
		include_directories(${${PACKAGE}_INCLUDE_DIRS})
		message(STATUS "${PACKAGE} includes: ${${PACKAGE}_INCLUDE_DIRS}")
		message(STATUS "${PACKAGE} libs: ${${PACKAGE}_LIBRARIES}")
	endif()
endforeach()

find_package(YAPP)

#packaging
include(CPackConfig.txt)

if(HAVE_LIBPYTHON)
	set(HAVE_PYTHON 1)
	set(PYTHON_DIR "${CMAKE_INSTALL_PREFIX}/lib/wireshark/python/${CPACK_PACKAGE_VERSION}")
endif()
if(HAVE_LIBLUA)
	set(HAVE_LUA_H 1)
	set(HAVE_LUA_5_1 1)
endif()
if(HAVE_LIBKERBEROS)
	set(HAVE_KERBEROS 1)
	# HAVE_HEIMDAL_KERBEROS
	set(HAVE_MIT_KERBEROS 1)
	set(HAVE_KEYTYPE_ARCFOUR_56 1)
endif()
if(HAVE_LIBGEOIP)
	set(HAVE_GEOIP 1)
endif()
if(HAVE_LIBCARES)
	set(HAVE_C_ARES 1)
endif()
if(HAVE_LIBADNS)
	set(HAVE_GNU_ADNS 1)
endif()
if(ENABLE_AIRPCAP)
	set(HAVE_AIRPCAP 1)
endif()
# That's the name autofoo uses
if(HAVE_LIBZLIB)
	set(HAVE_LIBZ 1)
endif()

include(ConfigureChecks.cmake)

#Big or little endian ?
include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

set(DATAFILE_DIR "${CMAKE_INSTALL_PREFIX}/share/${CPACK_PACKAGE_NAME}")

if(ENABLE_PLUGINS)
	set(HAVE_PLUGINS 1)
	set(PLUGIN_DIR="${DATAFILE_DIR}/plugins/${CPACK_PACKAGE_VERSION}")
	set(PLUGIN_SRC_DIRS
		plugins/asn1
		plugins/docsis
		plugins/ethercat
		plugins/giop
		plugins/gryphon
		plugins/interlink
		plugins/irda
		plugins/m2m
		plugins/mate
		plugins/opcua
		plugins/profinet
		plugins/sercosiii
		plugins/stats_tree
		plugins/unistim
		plugins/wimax
		plugins/wimaxasncp
	)
# It seems this stuff doesn't build with autofoo either...
#	if(YAPP_FOUND)
#		set(PLUGIN_SRC_DIRS
#			${PLUGIN_SRC_DIRS}
#			plugins/tpg
#		)
#	endif()
else()
	set(PLUGIN_SRC_DIRS )
endif()

foreach(PLUGIN_DIR ${PLUGIN_SRC_DIRS})
	add_subdirectory( ${PLUGIN_DIR} )
endforeach()

add_subdirectory( codecs )
add_subdirectory( epan )
add_subdirectory( gtk )
add_subdirectory( tools/lemon )
add_subdirectory( wiretap )
add_subdirectory( wsutil )

if(ENABLE_GUIDES)
	add_subdirectory( docbook )
endif()

find_package(POD)

#Platform specific
if(UNIX)
	set(WS_VAR_IMPORT "extern")
endif()

if(APPLE)
#TODO verify that APPLE implies HAVE_OS_X_FRAMEWORKS
	set(HAVE_OS_X_FRAMEWORKS 1)
	FIND_LIBRARY (APPLE_CORE_SERVICES_LIBRARY CoreServices)
	FIND_LIBRARY (APPLE_COCOA_LIBRARY Cocoa)
endif()

if(WIN32)
	add_definitions(-D_NEED_VAR_IMPORT_)
	set(WS_VAR_IMPORT "__declspec(dllimport) extern")

	# Disable deprecation
	if(MSVC80 OR MSVC90)
		add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
	endif()
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmakeconfig.h.in ${CMAKE_BINARY_DIR}/config.h)

include(FeatureSummary)
#SET_FEATURE_INFO(NAME DESCRIPTION [URL [COMMENT] ]
PRINT_ENABLED_FEATURES()
PRINT_DISABLED_FEATURES()

link_directories(
	gtk
	codecs
	epan
	wiretap
	wsutil
)

ADD_CUSTOM_TARGET(
	svnversion ALL
	COMMAND ${PERL}
		${CMAKE_CURRENT_SOURCE_DIR}/make-version.pl
		${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/make-version.pl
)

ADD_CUSTOM_COMMAND(
	OUTPUT	svnversion.h
	COMMAND ${PERL}
		${CMAKE_CURRENT_SOURCE_DIR}/make-version.pl
		${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/make-version.pl
)

register_tap_files(tshark-tap-register.c
	${TSHARK_TAP_SRC}
)

ADD_CUSTOM_COMMAND(
	OUTPUT	ps.c
	COMMAND	${PYTHON_EXECUTABLE}
		${CMAKE_CURRENT_SOURCE_DIR}/tools/rdps.py
		${CMAKE_CURRENT_SOURCE_DIR}/print.ps
		ps.c
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/tools/rdps.py
		${CMAKE_CURRENT_SOURCE_DIR}/print.ps
)

if(UNIX)
	set(PLATFORM_SRC
		capture-pcap-util-unix.c
	)
endif()

if(WIN32)
	set(PLATFORM_SRC
		capture-wpcap.c capture_wpcap_packet.c
	)
endif()

set(WTAP_PLUGIN_SOURCES
	epan/plugins.c
	epan/report_err.c
	epan/filesystem.c
)

set(SHARK_COMMON_CAPTURE_SRC
	capture_errs.c
	capture_ifinfo.c
	capture_ui_utils.c
)

set(WIRESHARK_COMMON_SRC
	${PLATFORM_SRC}
	capture-pcap-util.c
	cfile.c
	clopts_common.c
	disabled_protos.c
	packet-range.c
	print.c
	ps.c
	svnversion.h
	sync_pipe_write.c
	timestats.c
	util.c
	tap-megaco-common.c
	tap-rtp-common.c
	version_info.c
)

set(TSHARK_TAP_SRC
	tap-afpstat.c
	tap-ansi_astat.c
	tap-bootpstat.c
	tap-camelcounter.c
	tap-camelsrt.c
	tap-comparestat.c
	tap-dcerpcstat.c
	tap-diameter-avp.c
	tap-funnel.c
	tap-gsm_astat.c
	tap-h225counter.c
	tap-h225rassrt.c
	tap-httpstat.c
	tap-iostat.c
	tap-iousers.c
	tap-mgcpstat.c
	tap-megacostat.c
	tap-protocolinfo.c
	tap-protohierstat.c
	tap-radiusstat.c
	tap-rpcstat.c
	tap-rpcprogs.c
	tap-rtp.c
	tap-sctpchunkstat.c
	tap-sipstat.c
	tap-smbsids.c
	tap-smbstat.c
	tap-stats_tree.c
	tap-sv.c
	tap-wspstat.c
)

set(INSTALL_DIRS
	diameter
	dtds
	help
	radius
	tpncp
	wimaxasncp
)

set(INSTALL_FILES
	${CMAKE_BINARY_DIR}/AUTHORS-SHORT
	COPYING
	${CMAKE_BINARY_DIR}/capinfos.html
	cfilters
	colorfilters
	dfilters
	${CMAKE_BINARY_DIR}/dumpcap.html
	${CMAKE_BINARY_DIR}/editcap.html
	${CMAKE_BINARY_DIR}/idl2wrs.html
	ipmap.html
	manuf
	${CMAKE_BINARY_DIR}/mergecap.html
	${CMAKE_BINARY_DIR}/rawshark.html
	services
	smi_modules
	${CMAKE_BINARY_DIR}/text2pcap.html
	${CMAKE_BINARY_DIR}/tshark.html
	${CMAKE_BINARY_DIR}/wireshark-filter.html
	${CMAKE_BINARY_DIR}/wireshark.html
	docbook/ws.css
)

set(LIBEPAN_LIBS
#		@NSL_LIBS@	# -lnsl
#		@SOCKET_LIBS@	# -lsocket
#		@SSL_LIBS@	# -lcrypto
		epan
#		$(plugin_ldadd)	# in case of static
		${PCRE_LIBRARIES}
		${PCAP_LIBRARIES}
		${CARES_LIBRARIES}
		${ADNS_LIBRARIES}
		${KERBEROS_LIBRARIES}
		${LUA_LIBRARIES}
		${PYTHON_LIBRARIES}
		${GEOIP_LIBRARIES}
		${GCRYPT_LIBRARIES}
		${GNUTLS_LIBRARIES}
		${SMI_LIBRARIES}
		${ZLIB_LIBRARIES}
		${M_LIBRARIES}
)

if(BUILD_wireshark)
	set(wireshark_LIBS
		ui
		${GTK2_LIBRARIES}
		codecs
		${PORTAUDIO_LIBRARIES}
		${LIBEPAN_LIBS}
		${APPLE_CORE_SERVICES_LIBRARY}
		${APPLE_COCOA_LIBRARY}
	)
	set(wireshark_FILES
		airpcap_loader.c
		alert_box.c
		capture.c
		capture_info.c
		capture_opts.c
		capture_sync.c
		color_filters.c
		file.c
		fileset.c
		filters.c
		g711.c
		merge.c
		proto_hier_stats.c
		summary.c
		tempfile.c
		${SHARK_COMMON_CAPTURE_SRC}
		${WIRESHARK_COMMON_SRC}
	)
	add_executable(wireshark ${wireshark_FILES})
	add_dependencies(wireshark svnversion)
	set_target_properties(wireshark PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(wireshark ${wireshark_LIBS})
	install(TARGETS wireshark RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_tshark)
	set(tshark_LIBS
		${LIBEPAN_LIBS}
		${APPLE_COCOA_LIBRARY}
	)
	set(tshark_FILES
		capture_opts.c
		capture_sync.c
		tempfile.c
		tshark-tap-register.c
		tshark.c
		${TSHARK_TAP_SRC}
		${SHARK_COMMON_CAPTURE_SRC}
	        ${WIRESHARK_COMMON_SRC}
	)
	add_executable(tshark ${tshark_FILES})
	add_dependencies(tshark svnversion)
	set_target_properties(tshark PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(tshark ${tshark_LIBS})
	install(TARGETS tshark RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_rawshark)
	set(rawshark_LIBS
		${LIBEPAN_LIBS}
		${APPLE_COCOA_LIBRARY}
	)
	set(rawshark_FILES
	        ${WIRESHARK_COMMON_SRC}
		rawshark.c
	)
	add_executable(rawshark ${rawshark_FILES})
	add_dependencies(rawshark svnversion)
	set_target_properties(rawshark PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(rawshark ${rawshark_LIBS})
	install(TARGETS rawshark RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_dftest)
	set(dftest_LIBS
		${LIBEPAN_LIBS}
	)
	set(dftest_FILES
		dftest.c
		util.c
	)
	add_executable(dftest ${dftest_FILES})
	set_target_properties(dftest PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(dftest ${dftest_LIBS})
	install(TARGETS dftest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_randpkt)
	set(randpkt_LIBS
		wiretap
		${M_LIBRARIES}
		${PCAP_LIBRARIES}
#		@SOCKET_LIBS@
#		@NSL_LIBS@
		${CARES_LIBRARIES}
		${ADNS_LIBRARIES}
		${ZLIB_LIBRARIES}
	)
	set(randpkt_FILES
		randpkt.c
	)
	add_executable(randpkt ${randpkt_FILES})
	set_target_properties(randpkt PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(randpkt ${randpkt_LIBS})
	install(TARGETS randpkt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_text2pcap)
	set(text2pcap_LIBS
		wiretap
		${M_LIBRARIES}
		${ZLIB_LIBRARIES}
	)
	set(text2pcap_CLEAN_FILES
		text2pcap.c
	)
	set(text2pcap_FILES
		${text2pcap_CLEAN_FILES}
	)
	add_lex_files(text2pcap_FILES
		text2pcap-scanner.l
	)
	add_executable(text2pcap ${text2pcap_FILES})
	add_dependencies(text2pcap svnversion)
	set_target_properties(text2pcap PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(text2pcap ${text2pcap_LIBS})
	install(TARGETS text2pcap RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_mergecap)
	set(mergecap_LIBS
		wiretap
		${ZLIB_LIBRARIES}
	)
	set(mergecap_FILES
		mergecap.c
		merge.c
		svnversion.h
	)
	add_executable(mergecap ${mergecap_FILES})
	add_dependencies(mergecap svnversion)
	set_target_properties(mergecap PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(mergecap ${mergecap_LIBS})
	install(TARGETS mergecap RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_capinfos)
	set(capinfos_LIBS
		wiretap
		${ZLIB_LIBRARIES}
		${GCRYPT_LIBRARIES}
		${APPLE_COCOA_LIBRARY}
	)
	set(capinfos_FILES
		capinfos.c
		${WTAP_PLUGIN_SOURCES}
	)
	add_executable(capinfos ${capinfos_FILES})
	set_target_properties(capinfos PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(capinfos ${capinfos_LIBS})
	install(TARGETS capinfos RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_editcap)
	set(editcap_LIBS
		wiretap
		${ZLIB_LIBRARIES}
	)
	set(editcap_FILES
		editcap.c
		epan/crypt/crypt-md5.c
		epan/nstime.c
		${WTAP_PLUGIN_SOURCES}
	)
	add_executable(editcap ${editcap_FILES})
	add_dependencies(editcap svnversion)
	set_target_properties(editcap PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(editcap ${editcap_LIBS})
	install(TARGETS editcap RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_dumpcap)
	set(dumpcap_LIBS
		wiretap
#		@INET_NTOP_LO@
		${PCAP_LIBRARIES}
		${CAP_LIBRARIES}
#		@SOCKET_LIBS@
#		@NSL_LIBS@
		${GCRYPT_LIBRARIES}
		${GNUTLS_LIBRARIES}
		${ZLIB_LIBRARIES}
		${APPLE_COCOA_LIBRARY}
	)
	set(dumpcap_FILES
		svnversion.h
		capture_opts.c
		capture-pcap-util.c
		capture_stop_conditions.c
		clopts_common.c
		conditions.c
		dumpcap.c
		pcapio.c
		ringbuffer.c
		sync_pipe_write.c
		tempfile.c
		version_info.c
		${PLATFORM_SRC}
	)
	add_executable(dumpcap ${dumpcap_FILES})
	add_dependencies(dumpcap svnversion)
	set_target_properties(dumpcap PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})
	target_link_libraries(dumpcap ${dumpcap_LIBS})
	install(TARGETS dumpcap RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

ADD_CUSTOM_COMMAND(
	OUTPUT	AUTHORS-SHORT
	COMMAND ${PERL}
		${CMAKE_SOURCE_DIR}/doc/perlnoutf.pl
		${CMAKE_SOURCE_DIR}/doc/make-authors-short.pl
		< ${CMAKE_SOURCE_DIR}/AUTHORS
		> AUTHORS-SHORT
	DEPENDS
		${CMAKE_SOURCE_DIR}/doc/perlnoutf.pl
		${CMAKE_SOURCE_DIR}/doc/make-authors-short.pl
		${CMAKE_SOURCE_DIR}/AUTHORS
)

ADD_CUSTOM_COMMAND(
	OUTPUT	AUTHORS-SHORT-FORMAT
		wireshark.pod
	COMMAND ${PERL}
		${CMAKE_SOURCE_DIR}/doc/perlnoutf.pl
		${CMAKE_SOURCE_DIR}/doc/make-authors-format.pl
		< AUTHORS-SHORT
		> AUTHORS-SHORT-FORMAT
	COMMAND cat
		${CMAKE_SOURCE_DIR}/doc/wireshark.pod.template
		AUTHORS-SHORT-FORMAT
		> wireshark.pod
	DEPENDS
		${CMAKE_SOURCE_DIR}/doc/perlnoutf.pl
		${CMAKE_SOURCE_DIR}/doc/make-authors-format.pl
		AUTHORS-SHORT
		${CMAKE_SOURCE_DIR}/doc/wireshark.pod.template
)

ADD_CUSTOM_COMMAND(
	OUTPUT	wireshark-filter.pod
	COMMAND tshark
		-G fields |
		${PERL}
		${CMAKE_SOURCE_DIR}/doc/dfilter2pod.pl
		${CMAKE_SOURCE_DIR}/doc/wireshark-filter.pod.template
		> wireshark-filter.pod
	DEPENDS
		${CMAKE_SOURCE_DIR}/doc/dfilter2pod.pl
		${CMAKE_SOURCE_DIR}/doc/wireshark-filter.pod.template
		tshark
)

pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/capinfos 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/dumpcap 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/editcap 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/idl2wrs 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/mergecap 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/rawshark 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/text2pcap 1 )
pod2manhtml( ${CMAKE_SOURCE_DIR}/doc/tshark 1 )
pod2manhtml( wireshark 1 )
pod2manhtml( wireshark-filter 4 )

add_custom_target(
	auxiliary ALL
	DEPENDS
		AUTHORS-SHORT
		capinfos.html
		dumpcap.html
		editcap.html
		idl2wrs.html
		mergecap.html
		rawshark.html
		text2pcap.html
		tshark.html
		wireshark.html
		wireshark-filter.html
)

set(MAN1_FILES
	${CMAKE_BINARY_DIR}/capinfos.1
	${CMAKE_BINARY_DIR}/dumpcap.1
	${CMAKE_BINARY_DIR}/editcap.1
	${CMAKE_BINARY_DIR}/idl2wrs.1
	${CMAKE_BINARY_DIR}/mergecap.1
	${CMAKE_BINARY_DIR}/rawshark.1
	${CMAKE_BINARY_DIR}/text2pcap.1
	${CMAKE_BINARY_DIR}/tshark.1
	${CMAKE_BINARY_DIR}/wireshark.1
)

set(MAN4_FILES
	${CMAKE_BINARY_DIR}/wireshark-filter.4
)

set(CLEAN_FILES
	${wireshark_FILES}
	${tshark_FILES}
	${rawshark_FILES}
	${dftest_FILES}
	${randpkt_FILES}
	${text2pcap_CLEAN_FILES}
	${mergecap_FILES}
	${capinfos_FILES}
	${editcap_FILES}
	${dumpcap_FILES}
)

if (WERROR)
	set_source_files_properties(
		${CLEAN_FILES}
		PROPERTIES
		COMPILE_FLAGS -Werror
	)
endif()

install(
	FILES
		${INSTALL_FILES}
	DESTINATION
		${CMAKE_INSTALL_DATADIR}/${CPACK_PACKAGE_NAME}
)

install(
	FILES
		${MAN1_FILES}
	DESTINATION
		${CMAKE_INSTALL_MANDIR}/man1
)

install(
	FILES
		${MAN4_FILES}
	DESTINATION
		${CMAKE_INSTALL_MANDIR}/man4
)

install(
	DIRECTORY
		${INSTALL_DIRS}
	DIRECTORY_PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ
	DESTINATION
		${CMAKE_INSTALL_DATADIR}/${CPACK_PACKAGE_NAME}
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile.*" EXCLUDE
	PATTERN "faq.py" EXCLUDE
)
