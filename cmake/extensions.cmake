message("** Processing extensions **")

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(SUBDIRS "${al_path}/extensions/al_ext")

set(EXTENSIONS_INCLUDE_DIRS "${al_path}/extensions")

foreach(subdir ${SUBDIRS})
  if (EXISTS "${al_path}/extensions/al_ext/${subdir}/extension.cmake")
#    message("Building extension: ${subdir}")
    include("${al_path}/extensions/al_ext/${subdir}/extension.cmake")
    list(APPEND EXTENSIONS_LIBRARIES ${CURRENT_EXTENSION_LIBRARIES})
    list(APPEND EXTENSIONS_INCLUDE_DIRS ${CURRENT_EXTENSION_INCLUDE_DIRS})
    list(APPEND EXTENSIONS_DEFINITIONS ${CURRENT_EXTENSION_DEFINITIONS})
#    message("--**-- ${subdir} : ${CURRENT_EXTENSION_LIBRARIES}")
  endif()
endforeach()

#message("---- ${EXTENSION_LIBRARIES}")

