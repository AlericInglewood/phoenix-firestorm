# -*- cmake -*-

project(media_plugin_libvlc)

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLWindow)
include(Linking)
include(PluginAPI)
include(MediaPluginBase)   # <FS:Zi> Linux volume catcher

include(LibVLCPlugin)

### media_plugin_libvlc

set(media_plugin_libvlc_SOURCE_FILES
    media_plugin_libvlc.cpp
    )

add_library(media_plugin_libvlc
    SHARED
    ${media_plugin_libvlc_SOURCE_FILES}
    )

# <FS:aleric> Move this below the target declaration.
if(NOT ADDRESS_SIZE EQUAL 32)
  if(WINDOWS)
    ##add_definitions(/FIXED:NO)
  else(WINDOWS) # not windows therefore gcc LINUX and DARWIN
    set_property(TARGET media_plugin_libvlc PROPERTY POSITION_INDEPENDENT_CODE ON)     # <FS::Aleric> set POSITION_INDEPENDENT_CODE instead of adding -fPIC directly.
  endif(WINDOWS)
endif(NOT ADDRESS_SIZE EQUAL 32)

target_link_libraries(media_plugin_libvlc
        media_plugin_base
        ll::libvlc
)

if (WINDOWS)
  set_target_properties(
    media_plugin_libvlc
    PROPERTIES
    LINK_FLAGS "/MANIFEST:NO /NODEFAULTLIB:LIBCMT"
    )
endif (WINDOWS)

if (DARWIN)
  # Don't prepend 'lib' to the executable name, and don't embed a full path in the library's install name
  set_target_properties(
    media_plugin_libvlc
    PROPERTIES
    PREFIX ""
    BUILD_WITH_INSTALL_RPATH 1
    INSTALL_NAME_DIR "@executable_path"
    LINK_FLAGS "-exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/../base/media_plugin_base.exp"
  )

endif (DARWIN)

if( LINUX )
  set(CMAKE_SHARED_LINKER_FLAGS  "-Wl,--build-id -Wl,-rpath,'$ORIGIN:$ORIGIN/../../lib'")
endif()
