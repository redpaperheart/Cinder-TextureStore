if( NOT TARGET CinderTextureStore )
	get_filename_component( CINDER_TEXTURE_STORE_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE )

	list( APPEND CinderTextureStore_SRCS
		${CINDER_TEXTURE_STORE_SOURCE_PATH}/rph/TextureStore.h
		${CINDER_TEXTURE_STORE_SOURCE_PATH}/rph/TextureStore.cpp
		${CINDER_TEXTURE_STORE_SOURCE_PATH}/rph/ConcurrentDeque.h
		${CINDER_TEXTURE_STORE_SOURCE_PATH}/rph/ConcurrentMap.h
		${CINDER_TEXTURE_STORE_SOURCE_PATH}/rph/ConcurrentQueue.h
	)
	if(MSVC)
		foreach(source ${CinderTextureStore_SRCS})
			get_filename_component(source_path "${source}" PATH)
			string(REPLACE "/" "\\" source_path_msvc "${source_path}")
			file(RELATIVE_PATH source_path_msvc_rel "${CINDER_TEXTURE_STORE_SOURCE_PATH}" "${source_path_msvc}")
			source_group("${source_path_msvc_rel}" FILES "${source}")
		endforeach()
	endif()
	add_library( CinderTextureStore ${CinderTextureStore_SRCS} )

	target_include_directories( CinderTextureStore PUBLIC "${CINDER_TEXTURE_STORE_SOURCE_PATH}" )
	target_include_directories( CinderTextureStore SYSTEM BEFORE PUBLIC "${CINDER_PATH}/include" )

	if( NOT TARGET cinder )
		    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		    find_package( cinder REQUIRED PATHS
		        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( CinderTextureStore PRIVATE cinder )
	
endif()



