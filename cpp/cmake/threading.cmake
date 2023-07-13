if(MULTITHREADING)
    message(STATUS "Multithreading is enabled.")
    if(WASM)
        add_compile_options(--target=wasm32-wasi-threads -pthread)
        add_link_options(--target=wasm32-wasi-threads -pthread)
    endif()
    #add_compile_options(-fsanitize=thread)
    #add_link_options(-fsanitize=thread)
else()
    message(STATUS "Multithreading is disabled.")
    add_definitions(-DNO_MULTITHREADING)
    set(OMP_MULTITHREADING OFF)
endif()


# Find OpenMP
if(APPLE AND OMP_MULTITHREADING)
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

if(OMP_MULTITHREADING)
  find_package(OpenMP REQUIRED)
  message(STATUS "OMP multithreading is enabled.")
endif(OMP_MULTITHREADING)

if (OPENMP_FOUND)
    include_directories("${OPENMP_INCLUDES}")
    link_directories("${OPENMP_LIBRARIES}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    # set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    target_link_libraries(${_name} OpenMP::OpenMP_CXX)
endif(OPENMP_FOUND)


if(DISABLE_TBB)
    message(STATUS "Intel Thread Building Blocks is disabled.")
    add_definitions(-DNO_TBB)
else()
    find_package(TBB QUIET OPTIONAL_COMPONENTS tbb)
    if(${TBB_FOUND})
        message(STATUS "Intel Thread Building Blocks is enabled.")
    else()
        message(STATUS "Could not locate TBB.")
        add_definitions(-DNO_TBB)
    endif()
endif()
