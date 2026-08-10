cmake_minimum_required(VERSION 3.20)

#----------------------------------------------------------------------------------------------------------------------

set(CUDASYS $ENV{CUDASYS})

set(CMAKE_CUDA_TOOLKIT_ROOT_DIR ${CUDASYS})
set(CMAKE_CUDA_COMPILER "${CUDASYS}/bin/nvcc")

link_directories("${CUDASYS}/lib64")

find_package(CUDAToolkit 13.2 REQUIRED COMPONENTS 
             cudart cublas cusparse cufft curand cusolver cusparseLt cudss cufile cupti nccl nvshmem_host)

#----------------------------------------------------------------------------------------------------------------------

set(CAFFE2_USE_CUDNN ON)
set(CAFFE2_USE_CUSPARSELT ON)
set(CAFFE2_USE_CUFILE ON)
set(USE_CUDSS ON)     

#----------------------------------------------------------------------------------------------------------------------

set(CMAKE_C_COMPILER "gcc-15")
set(CMAKE_CXX_COMPILER "g++-15")
set(CMAKE_CUDA_HOST_COMPILER "g++-15")

#----------------------------------------------------------------------------------------------------------------------

project(QEDataProcessing LANGUAGES CXX CUDA)

#----------------------------------------------------------------------------------------------------------------------

set(nvtx3_dir "${CUDASYS}/NVTX/c")
include("${nvtx3_dir}/nvtxImportedTargets.cmake")

#----------------------------------------------------------------------------------------------------------------------

set(CUDA_TOOLKIT_ROOT_DIR "${CUDASYS}")
set(TORCHSYS $ENV{TORCHSYS})
set(Torch_DIR "${TORCHSYS}/share/cmake/Torch")
find_package(Torch REQUIRED)

#----------------------------------------------------------------------------------------------------------------------

find_package(ROOT 6.36 CONFIG REQUIRED)
separate_arguments(ROOT_EXE_LINKER_FLAGS)

#----------------------------------------------------------------------------------------------------------------------

#find_package(OpenMP)

#----------------------------------------------------------------------------------------------------------------------

find_package(yaml-cpp REQUIRED)
find_package(pugixml REQUIRED)

#----------------------------------------------------------------------------------------------------------------------

find_package(CGAL REQUIRED COMPONENTS Qt6 Core)

#----------------------------------------------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -Wall -O2 -fPIC -ffast-math -mavx -flarge-source-files")

#----------------------------------------------------------------------------------------------------------------------

set(CMAKE_CUDA_ARCHITECTURES "native")
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Wno-deprecated-gpu-targets")

#----------------------------------------------------------------------------------------------------------------------

file(GLOB DOS_gVel_Eps_source ${PROJECT_SOURCE_DIR}/DOS_gVel_Eps.cc)

#----------------------------------------------------------------------------------------------------------------------

add_executable(DOS_gVel_Eps ${DOS_gVel_Eps_source})
target_include_directories(DOS_gVel_Eps PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(DOS_gVel_Eps PRIVATE cxx_std_17 cuda_std_17)
target_link_libraries(DOS_gVel_Eps PUBLIC ${TORCH_LIBRARIES}  
                                          CUDA::cudart 
                                          CUDA::cublas 
                                          CUDA::cusparse 
                                          CUDA::cufft 
                                          CUDA::curand
                                          CUDA::cusolver
                                          CUDA::cupti
                                          nccl
                                          nvshmem_host
                                          cudnn
                                          cusparseLt
                                          cudss
                                          cufile
                                          nvtx3-cpp
                                          ${ROOT_LIBRARIES} 
                                          ${ROOT_EXE_LINKER_FLAGS}
                                          CGAL::CGAL
                                          CGAL::CGAL_Basic_viewer
                                          #OpenMP::OpenMP_CXX
                                          yaml-cpp
                                          pugixml::pugixml)

#----------------------------------------------------------------------------------------------------------------------

install(TARGETS DOS_gVel_Eps DESTINATION ${PROJECT_SOURCE_DIR}/install)

#----------------------------------------------------------------------------------------------------------------------

add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstall files with install_manifest.txt"
    COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
            install_manifest.txt to be uninstalled!
)

#----------------------------------------------------------------------------------------------------------------------