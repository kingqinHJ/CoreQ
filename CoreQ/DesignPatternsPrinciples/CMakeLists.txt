add_library(DesignPatternsPrinciples SRP.h
    OCP.h
    LSP.h
    ISP.h
    DIP.h
    LOD.h
    CRP.h)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /source-charset:utf-8")
endif()
