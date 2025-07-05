# -*- cmake -*-

project(media_plugin_base)

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLWindow)
include(Linking)
include(PluginAPI)


### media_plugin_base

set(media_plugin_base_SOURCE_FILES
    media_plugin_base.cpp
)

set(media_plugin_base_HEADER_FILES
    CMakeLists.txt

    media_plugin_base.h
)

add_library(media_plugin_base
    ${media_plugin_base_SOURCE_FILES}
        )

# <FS:aleric> Move this below the target declaration.
if(NOT ADDRESS_SIZE EQUAL 32)
  if(WINDOWS)
    ##add_definitions(/FIXED:NO)
  else(WINDOWS) # not windows therefore gcc LINUX and DARWIN
    set_property(TARGET media_plugin_base PROPERTY POSITION_INDEPENDENT_CODE ON)     # <FS::Aleric> set POSITION_INDEPENDENT_CODE instead of adding -fPIC directly.
  endif(WINDOWS)
endif(NOT ADDRESS_SIZE EQUAL 32)

target_link_libraries( media_plugin_base llplugin )
target_include_directories( media_plugin_base  INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR})
