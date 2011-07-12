project (pdfparser)

cmake_minimum_required(VERSION 2.8)

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if (Magic_FOUND)
	add_executable(pdftest pdftest.cpp)
	target_link_libraries(pdftest STATIC streams ${MAGIC_LIBRARIES})
endif (Magic_FOUND)

add_library(pdfstream STATIC pdfparser.cpp)
target_link_libraries(pdfstream streams)

add_executable(pdf 
    basicparsers.cpp 
    footer.cpp 
    xreftable.cpp

    streamwrapper.cpp 

    pdfparser.cpp

    pdf.cpp 
)
target_link_libraries(pdf streams)
