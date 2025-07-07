No changes are needed for the provided code. The build logs indicate an issue with the CMake configuration, specifically with missing CMakeLists.txt files in certain directories and missing package configuration files (likely DrogonConfig.cmake). These issues stem from the build system configuration and are unrelated to the code you've provided.

To resolve the build issues:
1. Ensure the specified dependencies are properly installed in the project's third-party directory
2. Add valid CMakeLists.txt files to the missing directories or their parent directories
3. Verify that the Drogon library is properly installed with CMake configuration files
4. Update the CMakeLists.txt to use the correct paths for the dependencies

These are configuration issues that should be addressed outside of the code file itself. The code appears to be correctly written and includes proper headers and unit tests for the PersonInfo class.