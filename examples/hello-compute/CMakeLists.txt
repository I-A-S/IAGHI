set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(hello-compute ${SRC_FILES})

target_link_libraries(hello-compute PRIVATE
        IAGHI
)