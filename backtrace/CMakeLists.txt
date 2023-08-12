#aux_source_directory(src SRC)
set(SRC src/backtrace.c src/main.c)
add_executable(backtrace ${SRC})
add_library(add SHARED src/add.c)
target_link_libraries(backtrace add)

install(TARGETS add DESTINATION app)
install(TARGETS backtrace DESTINATION app)