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
include CMakeFiles/terminal.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/terminal.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/terminal.dir/flags.make

CMakeFiles/terminal.dir/src/piPlayer.c.o: CMakeFiles/terminal.dir/flags.make
CMakeFiles/terminal.dir/src/piPlayer.c.o: ../src/piPlayer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bian/code/C/git/piPlayerShell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/terminal.dir/src/piPlayer.c.o"
	/usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/terminal.dir/src/piPlayer.c.o   -c /home/bian/code/C/git/piPlayerShell/src/piPlayer.c

CMakeFiles/terminal.dir/src/piPlayer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/terminal.dir/src/piPlayer.c.i"
	/usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/bian/code/C/git/piPlayerShell/src/piPlayer.c > CMakeFiles/terminal.dir/src/piPlayer.c.i

CMakeFiles/terminal.dir/src/piPlayer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/terminal.dir/src/piPlayer.c.s"
	/usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/bian/code/C/git/piPlayerShell/src/piPlayer.c -o CMakeFiles/terminal.dir/src/piPlayer.c.s

CMakeFiles/terminal.dir/lib/terminal.c.o: CMakeFiles/terminal.dir/flags.make
CMakeFiles/terminal.dir/lib/terminal.c.o: ../lib/terminal.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bian/code/C/git/piPlayerShell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/terminal.dir/lib/terminal.c.o"
	/usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/terminal.dir/lib/terminal.c.o   -c /home/bian/code/C/git/piPlayerShell/lib/terminal.c

CMakeFiles/terminal.dir/lib/terminal.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/terminal.dir/lib/terminal.c.i"
	/usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/bian/code/C/git/piPlayerShell/lib/terminal.c > CMakeFiles/terminal.dir/lib/terminal.c.i

CMakeFiles/terminal.dir/lib/terminal.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/terminal.dir/lib/terminal.c.s"
	/usr/bin/x86_64-linux-gnu-gcc-8 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/bian/code/C/git/piPlayerShell/lib/terminal.c -o CMakeFiles/terminal.dir/lib/terminal.c.s

# Object files for target terminal
terminal_OBJECTS = \
"CMakeFiles/terminal.dir/src/piPlayer.c.o" \
"CMakeFiles/terminal.dir/lib/terminal.c.o"

# External object files for target terminal
terminal_EXTERNAL_OBJECTS =

libterminal.a: CMakeFiles/terminal.dir/src/piPlayer.c.o
libterminal.a: CMakeFiles/terminal.dir/lib/terminal.c.o
libterminal.a: CMakeFiles/terminal.dir/build.make
libterminal.a: CMakeFiles/terminal.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bian/code/C/git/piPlayerShell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C static library libterminal.a"
	$(CMAKE_COMMAND) -P CMakeFiles/terminal.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/terminal.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/terminal.dir/build: libterminal.a

.PHONY : CMakeFiles/terminal.dir/build

CMakeFiles/terminal.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/terminal.dir/cmake_clean.cmake
.PHONY : CMakeFiles/terminal.dir/clean

CMakeFiles/terminal.dir/depend:
	cd /home/bian/code/C/git/piPlayerShell/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bian/code/C/git/piPlayerShell /home/bian/code/C/git/piPlayerShell /home/bian/code/C/git/piPlayerShell/build /home/bian/code/C/git/piPlayerShell/build /home/bian/code/C/git/piPlayerShell/build/CMakeFiles/terminal.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/terminal.dir/depend
