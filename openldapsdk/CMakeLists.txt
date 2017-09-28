CMAKE_MINIMUM_REQUIRED (VERSION 3.3.2)
PROJECT (ldapsamples)

IF(WIN32)
    # make the Release version create a PDB
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    # minimize the size or the resulting EXE
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
ENDIF()

INCLUDE_DIRECTORIES("inc")
LINK_DIRECTORIES("lib")

#https://stackoverflow.com/questions/14306642/adding-multiple-executables-in-cmake
file (GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/samples/*.c")
foreach(samplesourcefile ${APP_SOURCES})
	file(RELATIVE_PATH relpath "${CMAKE_CURRENT_SOURCE_DIR}/samples" ${samplesourcefile})
	string( REPLACE ".c" "" samplename ${relpath})

	ADD_EXECUTABLE(${samplename} ${samplesourcefile})
	TARGET_LINK_LIBRARIES(${samplename} libldap_r.lib liblber.lib ldap_ssl.lib ssleay32.lib nldapextd.lib)
endforeach(samplesourcefile ${APP_SOURCES})
