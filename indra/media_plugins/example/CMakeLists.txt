# -*- cmake -*-

project(media_plugin_example)

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLWindow)
include(Linking)
include(PluginAPI)

include(ExamplePlugin)

### media_plugin_example

set(media_plugin_example_SOURCE_FILES
    media_plugin_example.cpp
    )

add_library(media_plugin_example
    SHARED
    ${media_plugin_example_SOURCE_FILES}
    )

# <FS:aleric> Move this below the target declaration.
if(NOT ADDRESS_SIZE EQUAL 32)
  if(WINDOWS)
    ##add_definitions(/FIXED:NO)
  else(WINDOWS) # not windows therefore gcc LINUX and DARWIN
    set_property(TARGET media_plugin_example PROPERTY POSITION_INDEPENDENT_CODE ON)     # <FS::Aleric> set POSITION_INDEPENDENT_CODE instead of adding -fPIC directly.
  endif(WINDOWS)
endif(NOT ADDRESS_SIZE EQUAL 32)

target_link_libraries(media_plugin_example media_plugin_base )

if (WINDOWS)
  set_target_properties(
    media_plugin_example
    PROPERTIES
    LINK_FLAGS "/MANIFEST:NO /NODEFAULTLIB:LIBCMT"
    )
endif (WINDOWS)

if (DARWIN)
  # Don't prepend 'lib' to the executable name, and don't embed a full path in the library's install name
  set_target_properties(
    media_plugin_example
    PROPERTIES
    PREFIX ""
    BUILD_WITH_INSTALL_RPATH 1
    INSTALL_NAME_DIR "@executable_path"
    LINK_FLAGS "-exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/../base/media_plugin_base.exp"
  )

endif (DARWIN)
