macro(create_single_source_cgal_program_qt4 firstfile )

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${firstfile})
  
    set( all ${CMAKE_CURRENT_SOURCE_DIR}/${firstfile} )
    
    # remaining files
    foreach( i ${ARGN} )
      set( all ${all} ${CMAKE_CURRENT_SOURCE_DIR}/${i} ) 
    endforeach()

    get_filename_component(exe_name ${firstfile} NAME_WE)

    # UI files (Qt Designer files)
    qt4_wrap_ui( DT_UI_FILES ${exe_name}.ui )

    # qrc files (resources files, that contain icons, at least)"
    qt4_add_resources ( DT_RESOURCE_FILES ./${exe_name}.qrc )

    # use the Qt MOC preprocessor on classes that derives from QObject
    qt4_generate_moc( ${exe_name}.cpp ${exe_name}.moc )
    
    add_executable(${exe_name} ${all})
    
    add_to_cached_list( CGAL_EXECUTABLE_TARGETS ${exe_name} )

    # Link the executable to CGAL and third-party libraries
    if ( CGAL_AUTO_LINK_ENABLED )    

      target_link_libraries(${exe_name} ${CGAL_3RD_PARTY_LIBRARIES} )

    else()

      target_link_libraries(${exe_name} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} )

    endif()
  
  endif()
    
endmacro()
