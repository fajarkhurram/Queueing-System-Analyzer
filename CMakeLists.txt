cmake_minimum_required(VERSION 3.19)
project(QueueingSystemCalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(QueueingSystemCalculator
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    MM1.h
    MM1.cpp
    InputValidator.h
    InputValidator.cpp
    QueueResult.h
    QueueResult.cpp
    MMS.h
    MMS.cpp
    MG1.h
    MG1.cpp
    MGS.h
    MGS.cpp
    GG1.h
    GG1.cpp
    GGS.h
    GGS.cpp
    TimeUnitConverter.h
    TimeUnitConverter.cpp
    ServerStatusWidget.h
    ServerStatusWidget.cpp
    QueueVisualizationWidget.h
    QueueVisualizationWidget.cpp
)

target_link_libraries(QueueingSystemCalculator
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS QueueingSystemCalculator
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# qt_generate_deploy_app_script()'s OUTPUT_SCRIPT signature requires Qt >= 6.5;
# guard it so the project still configures cleanly on older Qt6 (e.g. 6.4.x).
if(QT_VERSION VERSION_GREATER_EQUAL 6.5)
    qt_generate_deploy_app_script(
        TARGET QueueingSystemCalculator
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
