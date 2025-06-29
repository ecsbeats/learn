# Defines the C++ library target
cc_library(
    name = "utils_lib",
    srcs = ["src/utils.cc"],
    hdrs = ["include/my_project/utils.h"],
    # This is the key fix. It tells Bazel that the "include" directory
    # should be treated as an include path root. This allows your
    # #include "my_project/utils.h" to work correctly.
    strip_include_prefix = "include",
)

# Defines the C++ executable target
cc_binary(
    name = "my_app",
    srcs = ["src/main.cc"],
    # Specifies the dependency on our library target
    deps = [":utils_lib"],
)
