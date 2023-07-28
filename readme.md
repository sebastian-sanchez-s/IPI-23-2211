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

#### Uso

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

Compilar y ejecutar:
```sh
make all NCOL=<nro_columnas> NROW=<nro_filas>
```

Crear archivo pdf (de ser posible)
```sh
make pdf NCOL=<nro_columnas> NROW=<nro_filas>
```
> Este comando crea el directorio `tex` y `pdf`.

Ejemplo de la organización del directorio.
```
ROOT
|-- main.c
|-- syt.c
|-- makefile
|-- obj/
|-- pdf/
|-- raw/
|-- tex/
|-- (...)
```

