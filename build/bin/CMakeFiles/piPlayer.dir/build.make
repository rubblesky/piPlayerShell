# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/bian/code/C/git/piPlayerShell

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bian/code/C/git/piPlayerShell/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/piPlayer.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/piPlayer.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/piPlayer.dir/flags.make

bin/CMakeFiles/piPlayer.dir/piPlayer.c.o: bin/CMakeFiles/piPlayer.dir/flags.make
bin/CMakeFiles/piPlayer.dir/piPlayer.c.o: ../src/piPlayer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bian/code/C/git/piPlayerShell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object bin/CMakeFiles/piPlayer.dir/piPlayer.c.o"
	cd /home/bian/code/C/git/piPlayerShell/build/bin && /usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/piPlayer.dir/piPlayer.c.o   -c /home/bian/code/C/git/piPlayerShell/src/piPlayer.c

bin/CMakeFiles/piPlayer.dir/piPlayer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/piPlayer.dir/piPlayer.c.i"
	cd /home/bian/code/C/git/piPlayerShell/build/bin && /usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/bian/code/C/git/piPlayerShell/src/piPlayer.c > CMakeFiles/piPlayer.dir/piPlayer.c.i

bin/CMakeFiles/piPlayer.dir/piPlayer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/piPlayer.dir/piPlayer.c.s"
	cd /home/bian/code/C/git/piPlayerShell/build/bin && /usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/bian/code/C/git/piPlayerShell/src/piPlayer.c -o CMakeFiles/piPlayer.dir/piPlayer.c.s

# Object files for target piPlayer
piPlayer_OBJECTS = \
"CMakeFiles/piPlayer.dir/piPlayer.c.o"

# External object files for target piPlayer
piPlayer_EXTERNAL_OBJECTS =

bin/piPlayer: bin/CMakeFiles/piPlayer.dir/piPlayer.c.o
bin/piPlayer: bin/CMakeFiles/piPlayer.dir/build.make
bin/piPlayer: lib/libterminal.a
bin/piPlayer: bin/CMakeFiles/piPlayer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bian/code/C/git/piPlayerShell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable piPlayer"
	cd /home/bian/code/C/git/piPlayerShell/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/piPlayer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/piPlayer.dir/build: bin/piPlayer

.PHONY : bin/CMakeFiles/piPlayer.dir/build

bin/CMakeFiles/piPlayer.dir/clean:
	cd /home/bian/code/C/git/piPlayerShell/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/piPlayer.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/piPlayer.dir/clean

bin/CMakeFiles/piPlayer.dir/depend:
	cd /home/bian/code/C/git/piPlayerShell/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bian/code/C/git/piPlayerShell /home/bian/code/C/git/piPlayerShell/src /home/bian/code/C/git/piPlayerShell/build /home/bian/code/C/git/piPlayerShell/build/bin /home/bian/code/C/git/piPlayerShell/build/bin/CMakeFiles/piPlayer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/piPlayer.dir/depend

