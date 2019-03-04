cmake_minimum_required(VERSION 2.8)

project(sca)

set(DEBUG "-g")
set(CFLAGS "-m64 -Wall -Werror -O3 -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")

message("\n\n")

find_library(OPENCL_LIB OpenCL PATHS 
	$ENV{AMDAPPSDKROOT}/lib/x86_64)

#find_library(CUDA_LIB cudart PATHS 
#	$ENV{CUDA}/lib64)


if (OPENCL_LIB)
	message("OpenCL library found: ${OPENCL_LIB}")
else (OPENCL_LIB)
	message("OpenCL library not found...")
endif (OPENCL_LIB)

#if (CUDA_LIB)
#	message("CUDA library found: ${CUDA_LIB}")
#else (CUDA_LIB)
#	message("CUDA library not found...")
#endif (CUDA_LIB)


find_path(OPENCL_INC CL/cl.h PATHS $ENV{AMDAPPSDKROOT}/include)

#find_path(CUDA_INC cuda.h PATHS $ENV{CUDA}/include)
find_package(CUDA QUIET)


if (OPENCL_INC)
	message("OpenCL headers found: ${OPENCL_INC}")
else (OPENCL_INC)
	message("OpenCL headers not found...")
endif (OPENCL_INC)

#if (CUDA_INC)
#	message("CUDA headers found: ${CUDA_INC}")
#else (CUDA_INC)
#	message("CUDA headers not found...")
#endif (CUDA_INC)


if (OPENCL_LIB AND OPENCL_INC)
	message("The OpenCL component of the tool will be built...")
	include_directories(${OPENCL_INC})
	add_definitions(-DOPENCL)
else (OPENCL_LIB AND OPENCL_INC)
	message("The OpenCL component of the tool will not be built...")
endif (OPENCL_LIB AND OPENCL_INC)

message("\n")

#if (CUDA_LIB AND CUDA_INC)
#	message("The CUDA component of the tool will be built...")
#	include_directories(${CUDA_INC})
#	add_definitions(-DCUDA)
#else (CUDA_LIB AND CUDA_INC)
#	message("The CUDA component of the tool will not be built...")
#endif (CUDA_LIB AND CUDA_INC)

if (CUDA_FOUND)
	message("CUDA package found")
	message("The CUDA component of the tool will be built...")
	add_definitions(-DCUDA)
	#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -O3)
else (CUDA)
	message("CUDA package not found")
	message("The CUDA component of the tool will not be built...")
endif (CUDA_FOUND)


add_subdirectory(common)
add_subdirectory(cpa)

set(sca_source_files sca.cpp)
add_executable(sca ${sca_source_files})


target_link_libraries(sca cpa)

if (OPENCL_LIB AND OPENCL_INC)
	target_link_libraries(sca ${OPENCL_LIB})
endif (OPENCL_LIB AND OPENCL_INC)

#if (CUDA_LIB AND CUDA_INC)
#	target_link_libraries(sca ${CUDA_LIB})
#endif (CUDA_LIB AND CUDA_INC)

message("\n")
