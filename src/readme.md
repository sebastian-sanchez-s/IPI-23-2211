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

Dentro del directorio `src/syt<K>/` ingrese los siguiente comandos para:

Compilar `sy<K>.c`:
```sh
make syt<K>
```
> Este comando crea el directorio `obj` con los ejecutables dentro.

Ejecutar:
```sh
make run M=<nro_columnas> N=<nro_filas>
```
> Este comando crea el directorio `raw` con los datos dentro.

Compilar y ejecutar:
```sh
make all M=<nro_columnas> N=<nro_filas>
```

Crear archivo pdf (de ser posible)
```sh
make pdf M=<nro_columnas> N=<nro_filas>
```

Ejemplo de la organización del directorio `src/syt<K>`.
```
src
|-- sytK.c
|-- makefile
|-- obj
|-- pdf
|-- raw
|-- tex
```

