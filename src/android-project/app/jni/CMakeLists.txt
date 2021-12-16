cmake_minimum_required(VERSION 3.6)

project(reflection-keen)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()


# SDL sources are in a subfolder/symlink named "SDL"
add_subdirectory(SDL)

# Reflection Keen sources are in a subfolder/symlink named "refkeen"
add_subdirectory(refkeen)

