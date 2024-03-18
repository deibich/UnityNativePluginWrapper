cmake_minimum_required(VERSION 3.26)

project(
    UnityNativePluginWrapperProject
    VERSION 1.0
    LANGUAGES CXX
)

set(UNITY_PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/UnityProject")

add_library(UnityPluginAPI INTERFACE)
target_include_directories(UnityPluginAPI
    INTERFACE
        "UnityPluginAPI"
)

add_library(UnityNativePluginWrapper SHARED wrapper.cpp)
target_link_libraries(UnityNativePluginWrapper PRIVATE UnityPluginAPI)
target_compile_features(UnityNativePluginWrapper PUBLIC cxx_std_20)
install(TARGETS UnityNativePluginWrapper DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/UnityNativePluginWrapper)

add_library(TestLib SHARED TestLib.cpp)
target_link_libraries(TestLib PRIVATE UnityPluginAPI)
target_compile_features(TestLib PUBLIC cxx_std_20)
install(TARGETS TestLib DESTINATION ${UNITY_PROJECT_ROOT_DIR}/Assets/UnityNativePluginWrapper/TestLib)
