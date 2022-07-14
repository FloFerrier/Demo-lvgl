find_path(MariaDBClient_INCLUDE_DIR NAMES mysql.h PATH_SUFFIXES mariadb mysql)

set(BAK_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
find_library(MariaDBClient_LIBRARY
    NAMES mariadb libmariadb mariadbclient libmariadbclient mysqlclient 
    libmysqlclient
    PATH_SUFFIXES mariadb mysql
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${BAK_CMAKE_FIND_LIBRARY_SUFFIXES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MariaDBClient DEFAULT_MSG 
MariaDBClient_LIBRARY MariaDBClient_INCLUDE_DIR)

if(MariaDBClient_FOUND)
    if(NOT TARGET MariaDBClient::MariaDBClient)
        add_library(MariaDBClient::MariaDBClient UNKNOWN IMPORTED)
        set_target_properties(MariaDBClient::MariaDBClient PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MariaDBClient_INCLUDE_DIR}"
            IMPORTED_LOCATION "${MariaDBClient_LIBRARY}")
    endif()
endif()

mark_as_advanced(MariaDBClient_INCLUDE_DIR MariaDBClient_LIBRARY)

set(MariaDBClient_LIBRARIES ${MariaDBClient_LIBRARY})
set(MariaDBClient_INCLUDE_DIRS ${MariaDBClient_INCLUDE_DIR})