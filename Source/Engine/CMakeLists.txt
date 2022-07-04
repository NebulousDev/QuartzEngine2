cmake_minimum_required(VERSION 3.20.0)

project(QuartzEngine)

include(GNUInstallDirs)

add_executable(${PROJECT_NAME}
    "Source/Quartz.cpp"

    "Source/Entity/EntityGraph.cpp"
    "Source/Entity/World.cpp"
    "Source/Entity/Component/Transform.cpp"

    "Source/System/System.cpp"
    "Source/System/SystemAdmin.cpp"
    "Source/System/DynamicLibrary.cpp"

    "Source/Windows/WinApiLibraryLoader.cpp"
    "Source/Windows/WinApiDynamicLibrary.cpp"
    "Source/Windows/WinApi.cpp"    
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
        ${QUARTZLIB_INCLUDE_PATH}
        ${QUARTZMATH_INCLUDE_PATH}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        QuartzApp
        QuartzLog
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Quartz")

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
