project(Transition Fortran)

cmake_minimum_required(VERSION 3.5.1)

cmake_policy(SET CMP0012 NEW) # if() recognizes boolean constants

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products" )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(STATIC_EXE FALSE)

# first create a static library of shared stuff
set(LIB_SRC
  DataGlobals.f90
  DataStringGlobals.f90
  DataVCompareGlobals.f90
  DisplayRoutines.f90
  General.f90
  InputProcessor.f90
  platformDepUtilityRoutines.f90
  SortAndStringUtilities.f90
  UtilityRoutines.f90
  VCompareGlobalRoutines.f90
  VCompareUtilityRoutines.f90
)
add_library( TransitionLib STATIC ${LIB_SRC} )

list( APPEND VERSIONS 1_0_0 )
list( APPEND VERSIONS 1_0_1 )
list( APPEND VERSIONS 1_0_2 )
list( APPEND VERSIONS 1_0_3 )
list( APPEND VERSIONS 1_1_0 )
list( APPEND VERSIONS 1_1_1 )
list( APPEND VERSIONS 1_2_0 )
list( APPEND VERSIONS 1_2_1 )
list( APPEND VERSIONS 1_2_2 )
list( APPEND VERSIONS 1_2_3 )
list( APPEND VERSIONS 1_3_0 )
list( APPEND VERSIONS 1_4_0 )
list( APPEND VERSIONS 2_0_0 )
list( APPEND VERSIONS 2_1_0 )
list( APPEND VERSIONS 2_2_0 )
list( APPEND VERSIONS 3_0_0 )
list( APPEND VERSIONS 3_1_0 )
list( APPEND VERSIONS 4_0_0 )
list( APPEND VERSIONS 5_0_0 )
list( APPEND VERSIONS 6_0_0 )
list( APPEND VERSIONS 7_0_0 )
list( APPEND VERSIONS 7_1_0 )
list( APPEND VERSIONS 7_2_0 )
list( APPEND VERSIONS 8_0_0 )
list( APPEND VERSIONS 8_1_0 )
list( APPEND VERSIONS 8_2_0 )
list( APPEND VERSIONS 8_3_0 )
list( APPEND VERSIONS 8_4_0 )
list( APPEND VERSIONS 8_5_0 )
list( APPEND VERSIONS 8_6_0 )
list( APPEND VERSIONS 8_7_0 )
list( APPEND VERSIONS 8_8_0 )
list( APPEND VERSIONS 8_9_0 )
list( APPEND VERSIONS 9_0_0 )
list( APPEND VERSIONS 9_1_0 )
list( APPEND VERSIONS 9_2_0 )
list( APPEND VERSIONS 9_3_0 )
list( APPEND VERSIONS 9_4_0 )
list( APPEND VERSIONS 9_5_0 )

if(APPLE)
  if( "${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU" )
    if(NOT "Ninja" STREQUAL ${CMAKE_GENERATOR})
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -cpp")
    endif()
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ffree-line-length-none")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fbounds-check")
  else()
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fpp")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static-intel")
  endif()
elseif(UNIX)
  set(CMAKE_SKIP_RPATH TRUE)
  if( "${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU" )
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static" )
    if(NOT "Ninja" STREQUAL ${CMAKE_GENERATOR})
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -cpp")
    endif()
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ffree-line-length-none")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fbounds-check")
    set(STATIC_EXE TRUE)
  else()
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fpp")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static-intel")
  endif()
else()  # Windows
  set(STATIC_EXE TRUE)
  if( "${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU" )
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static")
    if(NOT "Ninja" STREQUAL ${CMAKE_GENERATOR})
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -cpp")
    endif()
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ffree-line-length-none")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fbounds-check")
  else()
    # Set release flags to be empty
    set(CMAKE_Fortran_FLAGS_RELEASE "")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /libs:static")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /fpp")
  endif()
endif()

list( LENGTH VERSIONS VERSIONS_SIZE)

math(EXPR end "${VERSIONS_SIZE} - 1")

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ProjectMacros.cmake")

foreach( i RANGE 1 ${end} )

  math(EXPR ii "${i} - 1")

  list(GET VERSIONS ${ii} OLD_VERSION)
  list(GET VERSIONS ${i} NEW_VERSION)

  # then create all the binaries using just the Transition source and the appropriate version of the main sub
  set(SRC
    Transition.f90
    CreateNewIDFUsingRulesV${NEW_VERSION}.f90
  )

  string( REPLACE _ - OLD-VERSION ${OLD_VERSION} )
  string( REPLACE _ - NEW-VERSION ${NEW_VERSION} )

  set(LAST_NAME "Transition-V${OLD-VERSION}-to-V${NEW-VERSION}" )

  set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LAST_NAME} )
  add_executable( "${LAST_NAME}" ${SRC} )
  set_target_properties("${LAST_NAME}" PROPERTIES FOLDER Auxiliary)
  target_link_libraries( "${LAST_NAME}" TransitionLib )

  if( "${i}" GREATER "22" )
    install( TARGETS "${LAST_NAME}" DESTINATION PreProcess/IDFVersionUpdater )
    if(NOT ${STATIC_EXE})
      install_target_prereqs( "${LAST_NAME}" PreProcess/IDFVersionUpdater )
    endif()
  endif()

endforeach()

# then install all the support files

install(FILES "InputRulesFiles/Rules7-2-0-to-8-0-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-0-0-to-8-1-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-1-0-to-8-2-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-2-0-to-8-3-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-3-0-to-8-4-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-4-0-to-8-5-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-5-0-to-8-6-0.xls" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-6-0-to-8-7-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-7-0-to-8-8-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-8-0-to-8-9-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules8-9-0-to-9-0-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-0-0-to-9-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-1-0-to-9-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-2-0-to-9-3-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-3-0-to-9-4-0.md" DESTINATION "PreProcess/IDFVersionUpdater")

# output changes file first introduced with v8.5.0, so no older ones exist
install(FILES "OutputRulesFiles/OutputChanges8-4-0-to-8-5-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges8-5-0-to-8-6-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges8-6-0-to-8-7-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges8-7-0-to-8-8-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges8-8-0-to-8-9-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges8-9-0-to-9-0-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-0-0-to-9-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-1-0-to-9-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-2-0-to-9-3-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-3-0-to-9-4-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-4-0-to-9-5-0.md" DESTINATION "PreProcess/IDFVersionUpdater")

install(FILES "SupportFiles/Report Variables 7-2-0-006 to 8-0-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-0-0-007 to 8-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-1-0-009 to 8-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-2-0 to 8-3-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-3-0 to 8-4-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-4-0 to 8-5-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-5-0 to 8-6-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-6-0 to 8-7-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-7-0 to 8-8-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-8-0 to 8-9-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 8-9-0 to 9-0-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-0-0 to 9-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-1-0 to 9-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-2-0 to 9-3-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-3-0 to 9-4-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-4-0 to 9-5-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
