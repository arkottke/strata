cmake_minimum_required(VERSION 3.2)

include(UseLATEX.cmake)

# Note that normally CMakeLists.txt would not be considered an
# input to the document, but in this special case of documenting
# UseLATEX.cmake the contents of this file is actually included
# in the document.
add_latex_document(
    manual.tex
    BIBFILES references.bib
    IMAGE_DIRS figures
    FORCE_PDF
    EXCLUDE_FROM_ALL
    )
