# Coding Standards
First draft of the coding standards for the RSE

## File Naming
Files should always be named using Pascal case (SoundManager.xxx) and the header files should use (.hpp) for C++ code and (.h) for C code if necessary in the future, source files should use the usual (.cpp)

## General code 
**Namespaces** in the code should use a lowercase `namespace audio`
**Classes** in the code base should use Pascalcase `class SoundManager`
**Functions** in the code base should Camelcase `static Sound* getSound(const std::string& name)`