cmake_minimum_required(VERSION 3.20.0)

project(QuartzEngine)

include(GNUInstallDirs)

set(QUARTZENGINE_COMIPLE_DEFINITIONS)

set(QUARTZENGINE_SOURCE_FILES
	"Source/Quartz.cpp"
    "Source/Engine.cpp"
    "Source/Entity/EntityDatabase.cpp"
    "Source/Entity/EntityGraph.cpp"
    "Source/Entity/World.cpp"
    "Source/Module/Module.cpp"
    "Source/Module/ModuleRegistry.cpp"
    "Source/Module/DynamicLibrary.cpp"
    "Source/Runtime/Runtime.cpp"
    "Source/Runtime/Timer.cpp"
    "Source/Input/Input.cpp"
    "Source/Input/InputDevice.cpp"
    "Source/Input/InputDeviceRegistry.cpp"
    "Source/Config/Config.cpp"
    "Source/Filesystem/File.cpp"
    "Source/Filesystem/Folder.cpp"
    "Source/Filesystem/Filesystem.cpp"
)

if(WIN32)

	set(QUARTZENGINE_COMIPLE_DEFINITIONS ${QUARTZENGINE_COMIPLE_DEFINITIONS} QUARTZENGINE_WINAPI)
	set(QUARTZENGINE_WINAPI PARENT_SCOPE)

	set(QUARTZENGINE_SOURCE_FILES ${QUARTZENGINE_SOURCE_FILES}
		"Source/Module/Windows/WinApiLibraryLoader.cpp"
        "Source/Module/Windows/WinApiDynamicLibrary.cpp"
        "Source/Module/Windows/WinApi.cpp" 
	)

elseif(UNIX)

	set(QUARTZENGINE_COMIPLE_DEFINITIONS ${QUARTZENGINE_COMIPLE_DEFINITIONS} QUARTZENGINE_LINUX)
	set(QUARTZENGINE_LINUX PARENT_SCOPE)
	
	set(QUARTZENGINE_SOURCE_FILES ${QUARTZENGINE_SOURCE_FILES}
		"Source/Module/Linux/LinuxLibraryLoader.cpp"
        "Source/Module/Linux/LinuxDynamicLibrary.cpp"
	)

endif()

add_executable(${PROJECT_NAME} ${QUARTZENGINE_SOURCE_FILES} )

target_include_directories(${PROJECT_NAME}
    PUBLIC 
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
        ${QUARTZLIB_INCLUDE_PATH}
        ${QUARTZMATH_INCLUDE_PATH}
        ${QUARTZLOG_INCLUDE_PATH}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        QuartzApp
        QuartzCore
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_compile_definitions(${PROJECT_NAME} PUBLIC ${QUARTZENGINE_COMIPLE_DEFINITIONS})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Quartz" ENABLE_EXPORTS 1)

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
