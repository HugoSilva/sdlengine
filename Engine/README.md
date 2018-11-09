# Coding Standards
The following coding standards are a small guide how to keep the code base as coherent as possible, it does not cover all the use cases but it is a work in progress and should be updated at least every 6 months.

## File Naming
Files should always be named using Pascal case (SoundManager.xxx) and the header files should use (.hpp) for C++ code and (.h) for C code if necessary in the future, source files should use the usual (.cpp)

## General code 
**Namespaces** in the code should use a lowercase `namespace audio` I'm moving away from a section namespace in favor of a more general namespace for the engine, everything inside the engine should use `namespace rse`, in the future there might be a namespace for the editor as well

**Classes** in the code base should use Pascalcase `class SoundManager`

**Functions** in the code base should use Camelcase `static Sound* getSound(const std::string& name)`

**Variables** in the code base should use a simplified version of Hungarian Notation `int m_Volume`, there is no type description in the variable in this case the `n`

**Brackets** in the code base should always require a new line, for example `void Sound::resume()
	{
		Mix_Resume(m_Channel);
	}`, the only exception to this rule are inline declarations for example `inline const std::string& getName() const { return m_Name; }`

**C++11/14** in the code base should be mandatory when possible, raw pointers should be changed to be smart pointers, templates should be used when necessary, make use of range-based for loops, lambda and so one.