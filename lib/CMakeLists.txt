include_directories(include)

add_library(AliasToken SHARED
    Alias.cpp
    AliasToken.cpp
)
set_target_properties(AliasToken PROPERTIES
    SOVERSION 0
    COMPILE_FLAGS "-fno-rtti"
)

include(GNUInstallDirs)
install(TARGETS AliasToken
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
