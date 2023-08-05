# Contexto

Este repositorio contiene código generado
en la investigación de invierno IPI-23-2211

La idea es computar aquellas tablas de Young que
representan sumas exteriores.

## Standard Young Tableux (SYT)

Una tabla de Young en formato estándar consiste en
un arreglo 2-dimensional de números
de tal forma que cada fila y cada columna forme un
arreglo creciente de izquierda a derecha y de arriba
hacia abajo.

Este software computa las tablas de Young y guarda en
archivos separados cuales corresponden a sumas exteriores
(prefijo P) y cuáles no (prefijo N).

#### Uso

En linux (y posiblemente mac) hay que actualizar la ruta del linker
```sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/cddlib/lib-src/.libs/
```

Compilar:
```sh
make
```
> Este comando crea el directorio `obj` con el ejecutable dentro.

Ejecutar:
```sh
make run NCOL=<nro_columnas> NROW=<nro_filas>
```
> Este comando crea el directorio `raw` con los datos dentro.

Crear archivo pdf (de ser posible)
```sh
make pdf NCOL=<nro_columnas> NROW=<nro_filas>
```
> Este comando crea el directorio `tex` y `pdf`.

Ejemplo de la organización del directorio.
```
./IPI-23-2211
|-- main.c
|-- consumer.c
|-- producer.{h,c}
|-- queue.{h,c}
|-- util.h
|-- qsopt.h
|-- libqsopt.a
|-- makefile
|-- obj/
|-- pdf/
|-- raw/
|-- tex/
|-- (scripts y otros archivos) 
```

