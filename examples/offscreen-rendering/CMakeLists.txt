set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(offscreen-rendering ${SRC_FILES})

target_link_libraries(offscreen-rendering PRIVATE
        IAGHI
        glm::glm
)
