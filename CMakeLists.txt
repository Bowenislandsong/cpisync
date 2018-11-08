cmake_minimum_required(VERSION 3.6)
project(cpisynclib)
set(CMAKE_CXX_STANDARD 11)

# Parallel Programming OMP for MAC
if (APPLE)
set(CMAKE_C_COMPILER "/usr/local/Cellar/llvm/7.0.0/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/Cellar/llvm/7.0.0/bin/clang++")
set(OPENMP_LIBRARIES "/usr/local/Cellar/llvm/7.0.0/lib")
set(OPENMP_INCLUDES "/usr/local/Cellar/llvm/7.0.0/include")

OPTION (USE_OpenMP "Use OpenMP to enamble <omp.h>" ON)

# Find OpenMP
if(USE_OpenMP)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C "${CMAKE_C_COMPILER}")
        set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
        set(OpenMP_C_LIB_NAMES "libomp" "libgomp" "libiomp5")
        set(OpenMP_libomp_LIBRARY ${OpenMP_C_LIB_NAMES})
        set(OpenMP_libgomp_LIBRARY ${OpenMP_C_LIB_NAMES})
        set(OpenMP_libiomp5_LIBRARY ${OpenMP_C_LIB_NAMES})
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
      set(OpenMP_CXX_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
      set(OpenMP_CXX_LIB_NAMES "libomp" "libgomp" "libiomp5")
      set(OpenMP_libomp_LIBRARY ${OpenMP_CXX_LIB_NAMES})
      set(OpenMP_libgomp_LIBRARY ${OpenMP_CXX_LIB_NAMES})
      set(OpenMP_libiomp5_LIBRARY ${OpenMP_CXX_LIB_NAMES})
    endif()
endif()

if(USE_OpenMP)
  find_package(OpenMP REQUIRED)
endif(USE_OpenMP)

if (OPENMP_FOUND)
    include_directories("${OPENMP_INCLUDES}")
    link_directories("${OPENMP_LIBRARIES}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    # set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)

endif(APPLE)

# some flags
set(CMAKE_CXX_FLAGS "-DDEFAULT_LOGLEVEL=TEST -fopenmp")
#set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -Wall")
#set(CMAKE_LINKER_FLAGS_"-DDEFAULT_LOGLEVEL=TEST -fsanitize=address -fno-omit-frame-pointer -Wall")

# Set project directory strucuture
set(SRC_DIR src)
set(TEST_DIR tests)
set(UNIT_TEST_DIR ${TEST_DIR}/unit)
#set(SYSSHORT_TEST_DIR ${TEST_DIR}/sys/short)
set(SYSLONG_TEST_DIR ${TEST_DIR}/sys/long)
set(PERF_DIR ${TEST_DIR}/perf)
# Set location of the test runner
set(TEST_RUNNER ${TEST_DIR}/testRunner.cpp)

find_library(NTL_LIB ntl)
if(NOT NTL_LIB)
    message(FATAL_ERROR "NTL lib not found")
endif()
find_library(CPP_LIB cppunit)
if(NOT CPP_LIB)
    message(FATAL_ERROR "cppunit lib not found")
endif()
find_library(GMP_LIB gmp)
if(NOT GMP_LIB)
    message(FATAL_ERROR "GMP lib not found")
endif()


# Set file config
set(SOURCE_FILES
        ${SRC_DIR}/CommSocket.cpp
        ${SRC_DIR}/CommString.cpp
        ${SRC_DIR}/Communicant.cpp
        ${SRC_DIR}/CPISync.cpp
        ${SRC_DIR}/DataObject.cpp
        ${SRC_DIR}/GenSync.cpp
        ${SRC_DIR}/InterCPISync.cpp
        ${SRC_DIR}/Logger.cpp
        ${SRC_DIR}/probCPISync.cpp
        ${SRC_DIR}/SyncMethod.cpp
        ${SRC_DIR}/UID.cpp
		${SRC_DIR}/HashSync.cpp
        ${SRC_DIR}/CommDummy.cpp
        ${SRC_DIR}/IBLT.cpp
        ${SRC_DIR}/IBLTSync.cpp
        ${SRC_DIR}/FullSync.cpp
        ${SRC_DIR}/kshingling.cpp
        ${SRC_DIR}/kshinglingSync.cpp
        ${SRC_DIR}/UniqueDecode.cpp
        ${SRC_DIR}/AdjMtx.cpp
        ${SRC_DIR}/PerformanceData.cpp
        ${SRC_DIR}/IBLTSync_SetDiff.cpp
        ${SRC_DIR}/StrataEst.cpp
        ${SRC_DIR}/StrataEst_CPI.cpp
        ${SRC_DIR}/CPI.cpp tests/unit/CPITest.h)

#cppunit
include_directories(include)
include_directories(tests)


# Add libs and executables
add_library(cpisynclib ${SOURCE_FILES} )

# Define a macro for adding executables testing multiple files
# @param dir The relative path to the folder containing test files to add
# @param name The executable name
macro(add_group_test dir name)
    FILE(GLOB testPaths ${dir}/*Test.cpp ${dir}/*Tests.cpp)
    ADD_EXECUTABLE(${name} ${TEST_RUNNER} ${testPaths})

    FOREACH(test ${testPaths})
        GET_FILENAME_COMPONENT(testName ${test} NAME_WE)
        TARGET_LINK_LIBRARIES(${name} cpisynclib ntl cppunit gmp pthread)
        ADD_TEST(${testName} ${name})
    ENDFOREACH(test)
endmacro()

# Add test groups (note: executable will throw errors if there are no tests in the respective folder)
add_group_test(${UNIT_TEST_DIR} UnitTest)
#add_group_test(${SYSSHORT_TEST_DIR} SystemShortTests)
add_group_test(${SYSLONG_TEST_DIR} SystemLongTest)
add_group_test(${PERF_DIR} Performance)
