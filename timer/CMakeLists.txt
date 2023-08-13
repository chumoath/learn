aux_source_directory(src SRC)
add_executable(timer ${SRC})
install(TARGETS timer DESTINATION app)
