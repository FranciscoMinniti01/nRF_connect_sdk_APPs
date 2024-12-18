#
# DMQ WIRELESS PROJECT
#

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(wireless_dmq)

file(GLOB src_sources "src/*.c")                        # file(GLOB...) genera una lista llamada src_sources con todos los archivos fuente dentro de la carpeta src
target_sources(app PRIVATE ${src_sources})              # Asigna todos los archivos fuente encontrados a la aplicación.
