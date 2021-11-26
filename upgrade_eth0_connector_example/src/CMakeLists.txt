#Minimum required CMake Version
cmake_minimum_required(VERSION 3.6.1)

#Project Name
project(update_eth0_connector)
include_directories(../inc/)
link_directories(../lib/)
link_libraries(jsonc -lpthread)
set(CMAKE_C_COMPILER aarch64-poky-linux-gcc )
set(CMAKE_CXX_COMPILER aarch64-poky-linux-g++)

set(PROJECT_ALL_CODE_FILES update_eth0_connector.c)
ADD_EXECUTABLE(update_eth0_connector ${PROJECT_ALL_CODE_FILES})
target_link_libraries(update_eth0_connector)
