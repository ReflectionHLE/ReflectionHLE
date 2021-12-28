cmake_minimum_required(VERSION 3.6)

project(ReflectionHLE)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()


# SDL sources are in a subfolder/symlink named "SDL"
add_subdirectory(SDL)

# ReflectionHLE sources are in a subfolder/symlink named "ReflectionHLE"
add_subdirectory(ReflectionHLE)

