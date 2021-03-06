
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()

add_subdirectory( atom )
add_subdirectory( rdf )
add_subdirectory( rss2 )

include_directories( ${Boost_INCLUDE_DIR} )

add_definitions( -DQT_STRICT_ITERATORS )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

########### next target ###############

set(librss2local_SRCS
    rss2/category.cpp
    rss2/cloud.cpp
    rss2/document.cpp
    rss2/enclosure.cpp
    rss2/image.cpp
    rss2/item.cpp
    rss2/parser.cpp
    rss2/source.cpp
    rss2/textinput.cpp
    rss2/tools_p.cpp
	)


set(librdflocal_la_SRCS
    rdf/parser.cpp
    rdf/dublincorevocab.cpp
    rdf/dublincore.cpp
    rdf/syndicationvocab.cpp
    rdf/syndicationinfo.cpp
    rdf/contentvocab.cpp
    rdf/item.cpp
    rdf/image.cpp
    rdf/textinput.cpp
    rdf/document.cpp
    rdf/rssvocab.cpp
    rdf/node.cpp
    rdf/property.cpp
    rdf/literal.cpp
    rdf/resource.cpp
    rdf/model.cpp
    rdf/modelmaker.cpp
    rdf/rdfvocab.cpp
    rdf/sequence.cpp
    rdf/statement.cpp
    rdf/resourcewrapper.cpp
    rdf/nodevisitor.cpp
)

set(libatomlocal_SRCS
    atom/category.cpp
    atom/constants.cpp
    atom/content.cpp
    atom/document.cpp
    atom/entry.cpp
    atom/generator.cpp
    atom/link.cpp
    atom/parser.cpp
    atom/person.cpp
    atom/source.cpp
    atom/atomtools.cpp
)


set(libmapperrss2local_SRCS
    mapper/feedrss2impl.cpp
    mapper/itemrss2impl.cpp
    mapper/enclosurerss2impl.cpp
    mapper/categoryrss2impl.cpp
    mapper/imagerss2impl.cpp
)

set(libmapperrdflocal_SRCS
    mapper/feedrdfimpl.cpp
    mapper/itemrdfimpl.cpp
    mapper/imagerdfimpl.cpp
)

set(libmapperatomlocal_SRCS
    mapper/feedatomimpl.cpp
    mapper/itematomimpl.cpp
    mapper/enclosureatomimpl.cpp
    mapper/categoryatomimpl.cpp
)

set(syndication_LIB_SRCS
    ${librss2local_SRCS}
    ${librdflocal_la_SRCS}
    ${libatomlocal_SRCS}
    ${libmappercommonlocal_SRCS}
    ${libmapperrss2local_SRCS}
    ${libmapperrdflocal_SRCS}
    ${libmapperatomlocal_SRCS}
    documentsource.cpp
    documentvisitor.cpp
    elementwrapper.cpp
    constants.cpp
    feed.cpp
    item.cpp
    enclosure.cpp
    category.cpp
    image.cpp
    tools.cpp
    person.cpp
    personimpl.cpp
    loader.cpp
    global.cpp
    dataretriever.cpp
    specificitemvisitor.cpp
    specificitem.cpp
    specificdocument.cpp
    abstractparser.cpp
)


add_library(syndication ${LIBRARY_TYPE} ${syndication_LIB_SRCS})
generate_export_header(syndication EXPORT_FILE_NAME ksyndication_export.h)

target_link_libraries(syndication  ${KDE4_KIO_LIBS})

set_target_properties(syndication PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS syndication EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES abstractparser.h
               category.h
               constants.h
               dataretriever.h
               documentsource.h
               documentvisitor.h
               elementwrapper.h
               enclosure.h
               feed.h
               global.h
               image.h
               item.h
               ${CMAKE_CURRENT_BINARY_DIR}/ksyndication_export.h
               loader.h
               mapper.h
               parsercollection.h
               person.h
               specificdocument.h
               specificitem.h
               specificitemvisitor.h
               syndication.h
               tools.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/syndication COMPONENT Devel)
