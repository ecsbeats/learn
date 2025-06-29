add_library(Utils utils.cc)

target_include_directories(Utils PUBLIC ../include)

add_executable(MyApp main.cc)

target_link_libraries(MyApp PRIVATE Utils)
