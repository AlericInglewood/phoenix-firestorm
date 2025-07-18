# -*- cmake -*-

project(media_plugin_gstreamer010)

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLMath)
include(LLWindow)
include(Linking)
include(PluginAPI)
include(OpenGL)
include(MediaPluginBase)   # <FS:Zi> Linux volume catcher

include(GStreamer010Plugin)

### media_plugin_gstreamer010

set(media_plugin_gstreamer010_SOURCE_FILES
    media_plugin_gstreamer010.cpp
    llmediaimplgstreamer_syms.cpp
    llmediaimplgstreamervidplug.cpp
    )

set(media_plugin_gstreamer010_HEADER_FILES
    llmediaimplgstreamervidplug.h
    llmediaimplgstreamer_syms.h
    llmediaimplgstreamertriviallogging.h
    )

add_library(media_plugin_gstreamer010
    SHARED
    ${media_plugin_gstreamer010_SOURCE_FILES}
    )

# <FS:aleric> Move this below the target declaration.
if(NOT ADDRESS_SIZE EQUAL 32)
  if(WINDOWS)
    ##add_definitions(/FIXED:NO)
  else(WINDOWS) # not windows therefore gcc LINUX and DARWIN
    set_property(TARGET media_plugin_gstreamer010 PROPERTY POSITION_INDEPENDENT_CODE ON)     # <FS::Aleric> set POSITION_INDEPENDENT_CODE instead of adding -fPIC directly.
  endif(WINDOWS)
endif(NOT ADDRESS_SIZE EQUAL 32)

target_link_libraries(media_plugin_gstreamer010
        media_plugin_base
        ll::gstreamer
  )
