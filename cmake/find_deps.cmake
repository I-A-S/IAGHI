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

FetchContent_Declare(
        DearImGui_CMake
        GIT_REPOSITORY https://github.com/I-A-S/imgui-cmake
        GIT_TAG        main
        OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(volk VulkanMemoryAllocator DearImGui_CMake)

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

    FetchContent_Declare(
            STB_CMake
            GIT_REPOSITORY https://github.com/I-A-S/stb-cmake
            GIT_TAG        main
            OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(SDL glm STB_CMake)
endif()
