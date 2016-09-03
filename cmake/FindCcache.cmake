cmake_minimum_required(VERSION 2.8.12)

# Assuming here that ccache versions are 1 digit for major, 1 digit for minor, and a . in the middle
# and begin "ccache version X.Y.Z" ...

## Search for ccache
find_program(CCACHE ccache
             /bin
             /usr/bin
             /usr/local/bin
             /opt/local/bin)
if(CCACHE)
  ## Check CCACHE is working and check version

  execute_process(COMMAND ${CCACHE} "-V"
                  COMMAND "head" "-n1"
                  RESULT_VARIABLE CCACHE_RESULT
		  OUTPUT_VARIABLE CCACHE_OUTPUT
                  TIMEOUT 5)
  if(CCACHE_RESULT EQUAL 0)
    string(SUBSTRING ${CCACHE_OUTPUT} 15 5 CCACHE_VER)
    string(SUBSTRING ${CCACHE_VER} 0 1 CCACHE_MAJOR)
    string(SUBSTRING ${CCACHE_VER} 2 1 CCACHE_MINOR)

    MESSAGE(STATUS "CCACHE major version = " ${CCACHE_MAJOR})
    MESSAGE(STATUS "CCACHE minor version = " ${CCACHE_MINOR})

    if(MINGW)
      if((${CCACHE_MAJOR} EQUAL 3) AND (${CCACHE_MINOR} LESS 2))
        MESSAGE(WARNING "ccache is likely broken with mingw at this version due to a BUG!")
        MESSAGE(WARNING "ccache at version < 3.2 does not officially support GCC @ parameters")
        MESSAGE(WARNING "It is recommended to upgrade ccache to version >= 3.2.0!")
      endif()
    elseif(EMSCRIPTEN)
      if((${CCACHE_MAJOR} LESS 3) OR ((${CCACHE_MAJOR} EQUAL 3) AND (${CCACHE_MINOR} LESS 2)))
        MESSAGE(WARNING "ccache is likely broken with emscripten at this version due to a BUG!")
        MESSAGE(WARNING "old versions reject the '-s' option which is essential to emscripten")
        MESSAGE(WARNING "it is recommended to upgrade to >= 3.2.0, 3.2.2 is known to work")
      endif()
    endif()

    ## Enable ccache
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  else()
    MESSAGE(WARNING "'ccache -V' produced an error code: " ${RESULT_VARIABLE})
  endif()
endif()

