# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/fsl-imx-wayland/4.14-sumo/sysroots/x86_64-pokysdk-linux/usr/bin/cmake

# The command to remove a file.
RM = /opt/fsl-imx-wayland/4.14-sumo/sysroots/x86_64-pokysdk-linux/usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src

# Include any dependencies generated for this target.
include CMakeFiles/update_eth0_connector.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/update_eth0_connector.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/update_eth0_connector.dir/flags.make

CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o: CMakeFiles/update_eth0_connector.dir/flags.make
CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o: update_eth0_connector.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o"
	aarch64-poky-linux-gcc   --sysroot=/opt/fsl-imx-wayland/4.14-sumo/sysroots/aarch64-poky-linux $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o   -c /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src/update_eth0_connector.c

CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.i"
	aarch64-poky-linux-gcc   --sysroot=/opt/fsl-imx-wayland/4.14-sumo/sysroots/aarch64-poky-linux $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src/update_eth0_connector.c > CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.i

CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.s"
	aarch64-poky-linux-gcc   --sysroot=/opt/fsl-imx-wayland/4.14-sumo/sysroots/aarch64-poky-linux $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src/update_eth0_connector.c -o CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.s

CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.requires:

.PHONY : CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.requires

CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.provides: CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.requires
	$(MAKE) -f CMakeFiles/update_eth0_connector.dir/build.make CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.provides.build
.PHONY : CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.provides

CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.provides.build: CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o


# Object files for target update_eth0_connector
update_eth0_connector_OBJECTS = \
"CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o"

# External object files for target update_eth0_connector
update_eth0_connector_EXTERNAL_OBJECTS =

update_eth0_connector: CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o
update_eth0_connector: CMakeFiles/update_eth0_connector.dir/build.make
update_eth0_connector: CMakeFiles/update_eth0_connector.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable update_eth0_connector"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/update_eth0_connector.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/update_eth0_connector.dir/build: update_eth0_connector

.PHONY : CMakeFiles/update_eth0_connector.dir/build

CMakeFiles/update_eth0_connector.dir/requires: CMakeFiles/update_eth0_connector.dir/update_eth0_connector.c.o.requires

.PHONY : CMakeFiles/update_eth0_connector.dir/requires

CMakeFiles/update_eth0_connector.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/update_eth0_connector.dir/cmake_clean.cmake
.PHONY : CMakeFiles/update_eth0_connector.dir/clean

CMakeFiles/update_eth0_connector.dir/depend:
	cd /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src /home/ben/ben/4g_router/test/upgrade_eth0_connector_example/src/CMakeFiles/update_eth0_connector.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/update_eth0_connector.dir/depend

