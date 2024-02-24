include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(scoped_new_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(scoped_new_setup_options)
  option(scoped_new_ENABLE_HARDENING "Enable hardening" ON)
  option(scoped_new_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    scoped_new_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    scoped_new_ENABLE_HARDENING
    OFF)

  scoped_new_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR scoped_new_PACKAGING_MAINTAINER_MODE)
    option(scoped_new_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(scoped_new_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(scoped_new_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(scoped_new_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(scoped_new_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(scoped_new_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(scoped_new_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(scoped_new_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(scoped_new_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(scoped_new_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(scoped_new_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(scoped_new_ENABLE_PCH "Enable precompiled headers" OFF)
    option(scoped_new_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(scoped_new_ENABLE_IPO "Enable IPO/LTO" ON)
    option(scoped_new_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(scoped_new_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(scoped_new_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(scoped_new_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(scoped_new_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(scoped_new_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(scoped_new_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(scoped_new_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(scoped_new_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(scoped_new_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(scoped_new_ENABLE_PCH "Enable precompiled headers" OFF)
    option(scoped_new_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      scoped_new_ENABLE_IPO
      scoped_new_WARNINGS_AS_ERRORS
      scoped_new_ENABLE_USER_LINKER
      scoped_new_ENABLE_SANITIZER_ADDRESS
      scoped_new_ENABLE_SANITIZER_LEAK
      scoped_new_ENABLE_SANITIZER_UNDEFINED
      scoped_new_ENABLE_SANITIZER_THREAD
      scoped_new_ENABLE_SANITIZER_MEMORY
      scoped_new_ENABLE_UNITY_BUILD
      scoped_new_ENABLE_CLANG_TIDY
      scoped_new_ENABLE_CPPCHECK
      scoped_new_ENABLE_COVERAGE
      scoped_new_ENABLE_PCH
      scoped_new_ENABLE_CACHE)
  endif()

  scoped_new_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (scoped_new_ENABLE_SANITIZER_ADDRESS OR scoped_new_ENABLE_SANITIZER_THREAD OR scoped_new_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(scoped_new_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(scoped_new_global_options)
  if(scoped_new_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    scoped_new_enable_ipo()
  endif()

  scoped_new_supports_sanitizers()

  if(scoped_new_ENABLE_HARDENING AND scoped_new_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR scoped_new_ENABLE_SANITIZER_UNDEFINED
       OR scoped_new_ENABLE_SANITIZER_ADDRESS
       OR scoped_new_ENABLE_SANITIZER_THREAD
       OR scoped_new_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${scoped_new_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${scoped_new_ENABLE_SANITIZER_UNDEFINED}")
    scoped_new_enable_hardening(scoped_new_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(scoped_new_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(scoped_new_warnings INTERFACE)
  add_library(scoped_new_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  scoped_new_set_project_warnings(
    scoped_new_warnings
    ${scoped_new_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(scoped_new_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(scoped_new_options)
  endif()

  include(cmake/Sanitizers.cmake)
  scoped_new_enable_sanitizers(
    scoped_new_options
    ${scoped_new_ENABLE_SANITIZER_ADDRESS}
    ${scoped_new_ENABLE_SANITIZER_LEAK}
    ${scoped_new_ENABLE_SANITIZER_UNDEFINED}
    ${scoped_new_ENABLE_SANITIZER_THREAD}
    ${scoped_new_ENABLE_SANITIZER_MEMORY})

  set_target_properties(scoped_new_options PROPERTIES UNITY_BUILD ${scoped_new_ENABLE_UNITY_BUILD})

  if(scoped_new_ENABLE_PCH)
    target_precompile_headers(
      scoped_new_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(scoped_new_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    scoped_new_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(scoped_new_ENABLE_CLANG_TIDY)
    scoped_new_enable_clang_tidy(scoped_new_options ${scoped_new_WARNINGS_AS_ERRORS})
  endif()

  if(scoped_new_ENABLE_CPPCHECK)
    scoped_new_enable_cppcheck(${scoped_new_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(scoped_new_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    scoped_new_enable_coverage(scoped_new_options)
  endif()

  if(scoped_new_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(scoped_new_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(scoped_new_ENABLE_HARDENING AND NOT scoped_new_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR scoped_new_ENABLE_SANITIZER_UNDEFINED
       OR scoped_new_ENABLE_SANITIZER_ADDRESS
       OR scoped_new_ENABLE_SANITIZER_THREAD
       OR scoped_new_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    scoped_new_enable_hardening(scoped_new_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
