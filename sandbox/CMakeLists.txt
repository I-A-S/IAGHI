set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(Sandbox ${SRC_FILES})

target_link_libraries(Sandbox PRIVATE
        IAGHI
        glm::glm
        SDL3::SDL3
)

