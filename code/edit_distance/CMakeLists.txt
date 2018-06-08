cmake_minimum_required(VERSION 3.7)

project(editdistance)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCL REQUIRED)
find_package(MPI REQUIRED)

include_directories(${OpenCL_INCLUDE_DIR} ${MPI_CXX_INCLUDE_PATH})
link_directories(${OpenCL_LIBRARY} MPI_LIBRARY:FILEPATH)

set(SOURCE_FILES main.cpp utils.cpp)
add_executable(editdistance ${SOURCE_FILES})

target_link_libraries(editdistance ${OpenCL_LIBRARIES} ${MPI_CXX_LIBRARIES})

if(MPI_COMPILE_FLAGS)
	set_target_properties(editdistance PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
	set_target_properties(editdistance PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

add_custom_command(
        TARGET editdistance POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/kernel.cl"
        "$<TARGET_FILE_DIR:editdistance>/"
        COMMENT "Copying kernel to output directory"
)

add_custom_command(
        TARGET editdistance POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/sequences"
        "$<TARGET_FILE_DIR:editdistance>/sequences"
        COMMENT "Copying sequences to output directory"
)
