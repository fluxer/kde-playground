if (QT_QTDECLARATIVE_FOUND)
install(FILES
  qmldir
  collectionview.qml
  AkonadiBreadcrumbNavigationView.qml
  CollectionDelegate.qml
  border_dot.png
  check.png
  sliderbackground.png
  transparentplus.png
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/akonadi
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

endif ()
