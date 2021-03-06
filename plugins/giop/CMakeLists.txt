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

set(COSNAMING_SRC
	packet-cosnaming.c
)

set(COSEVENTCOMM_SRC
	packet-coseventcomm.c
)

set(TANGO_SRC
	packet-tango.c
)
set(PARLAY_SRC
	packet-parlay.c
)

set(CLEAN_FILES
	${COSNAMING_SRC}
	${COSEVENTCOMM_SRC}
	# Not clean ${TANGO_SRC}
	# Not clean ${PARLAY_SRC}
)

if (WERROR)
	set_source_files_properties(
		${CLEAN_FILES}
		PROPERTIES
		COMPILE_FLAGS -Werror
	)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(cosnaming ${LINK_MODE_MODULE}
	${COSNAMING_SRC}
)
set_target_properties(cosnaming PROPERTIES PREFIX "")
set_target_properties(cosnaming PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION})
set_target_properties(cosnaming PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})

target_link_libraries(cosnaming epan)

install(TARGETS cosnaming
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION} NAMELINK_SKIP
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
)

add_library(coseventcomm ${LINK_MODE_MODULE}
	${COSEVENTCOMM_SRC}
)
set_target_properties(coseventcomm PROPERTIES PREFIX "")
set_target_properties(coseventcomm PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION})
set_target_properties(coseventcomm PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})

target_link_libraries(coseventcomm epan)

install(TARGETS coseventcomm
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION} NAMELINK_SKIP
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
)

add_library(tango ${LINK_MODE_MODULE}
	${TANGO_SRC}
)
set_target_properties(tango PROPERTIES PREFIX "")
set_target_properties(tango PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION})
set_target_properties(tango PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})

target_link_libraries(tango epan)

install(TARGETS tango
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION} NAMELINK_SKIP
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
)

add_library(parlay ${LINK_MODE_MODULE}
	${PARLAY_SRC}
)
set_target_properties(parlay PROPERTIES PREFIX "")
set_target_properties(parlay PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION})
set_target_properties(parlay PROPERTIES LINK_FLAGS ${WS_LINK_FLAGS})

target_link_libraries(parlay epan)

install(TARGETS parlay
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION} NAMELINK_SKIP
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/@CPACK_PACKAGE_NAME@/plugins/${CPACK_PACKAGE_VERSION}
)

