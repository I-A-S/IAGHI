include(FetchContent)

FetchContent_Declare(
        volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG        master
        SYSTEM
        EXCLUDE_FROM_ALL
)

FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG        master
        SYSTEM
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(volk VulkanMemoryAllocator)

if(IAGHI_BUILD_UTILS)
    FetchContent_Declare(
            DearImGui_CMake
            GIT_REPOSITORY https://github.com/I-A-S/imgui-cmake
            GIT_TAG        main
            OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
            STB_CMake
            GIT_REPOSITORY https://github.com/I-A-S/stb-cmake
            GIT_TAG        main
            OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
            spirv-headers
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
            GIT_TAG        vulkan-sdk-1.4.341.0
            SYSTEM
            EXCLUDE_FROM_ALL
    )

    FetchContent_MakeAvailable(STB_CMake DearImGui_CMake spirv-headers)

    FetchContent_Declare(
            spirv-tools
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
            GIT_TAG        v2026.1
            SYSTEM
            EXCLUDE_FROM_ALL
    )
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
    set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(spirv-tools)

    FetchContent_Declare(
            glslang
            GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
            GIT_TAG        vulkan-sdk-1.4.341.0
            SYSTEM
            EXCLUDE_FROM_ALL
    )
    set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
    set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(glslang)
endif ()

if(IAGHI_BUILD_SANDBOX)
    FetchContent_Declare(
            SDL
            GIT_REPOSITORY https://github.com/libsdl-org/SDL
            GIT_TAG        main
            OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm
            GIT_TAG        master
            OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(SDL glm)
endif()
