# CoolSocket cross-library for Qt5

This is an implementation of CoolSocket for Qt5.

## Usage

This library can be linked against as a shared library. To use it, follow the instructions below.


* In terminal, `git submodule add https://github.com/libcoolsocket/qt5 PREFERRED_DIR`
* In your `CMakeLists.txt`, `add_subdirectory(PREFERRED_DIR)`
* And finally link against it by calling `target_link_libraries(EXE_OR_LIB_NAME PUBLIC coolsocket)`
