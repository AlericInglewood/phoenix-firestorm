# -*- cmake -*-

project(media_plugin_gstreamer10)

include(00-Common)
include(LLCommon)
include(LLImage)

include(LLMath)

include(LLWindow)
include(Linking)
include(PluginAPI)

include(OpenGL)
include(GLIB)

include(GStreamer10Plugin)


### media_plugin_gstreamer10

set(media_plugin_gstreamer10_SOURCE_FILES
    media_plugin_gstreamer10.cpp
    llmediaimplgstreamer_syms.cpp
    )

set(media_plugin_gstreamer10_HEADER_FILES
    llmediaimplgstreamer_syms.h
    llmediaimplgstreamertriviallogging.h
    )

add_library(media_plugin_gstreamer10
    SHARED
    ${media_plugin_gstreamer10_SOURCE_FILES}
)

# <FS:aleric> Move this below the target declaration.
if(NOT WORD_SIZE EQUAL 32)
  if(NOT WINDOWS) # not windows therefore gcc LINUX and DARWIN
    set_property(TARGET media_plugin_gstreamer10 PROPERTY POSITION_INDEPENDENT_CODE ON)     # <FS::Aleric> set POSITION_INDEPENDENT_CODE instead of adding -fPIC directly.
  endif()
endif(NOT WORD_SIZE EQUAL 32)

target_link_libraries(media_plugin_gstreamer10
        media_plugin_base
        ll::gstreamer
)


