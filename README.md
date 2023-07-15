# Contexto

Este repositorio contiene código generado
en la investigación de invierno IPI-23-2211

La pregunta de investigación es computar las
tablas de Young que representan sumas
exteriores.


## Standard Young Tableux (SYT)

Una tabla de Young en formato estándar consiste en
un arreglo 2-dimensional de números
de tal forma que cada fila y cada columna forme un
arreglo creciente de izquierda a derecha y de arriba
hacia abajo.

### Uso

Dentro del directorio `src`, para compilar y ejecutar se debe ingresar
```sh
make src=<archivo_a_compilar> M=<#columnas> N=<#filas>
```
> Para solo compilar no se debe ingresar M ni N

Esto genera dos directorios `obj` y `raw`, que contienen
el ejecutable y los datos generados por este, respectivamente.
Cada línea del archivo guardado en `raw` representa un arreglo
donde cada fila del arreglo está separa por `;` y cada elemento
de la fila está separada por `.`.

Para generar un pdf una vez generado los datos en `raw` se
debe ingresar
```sh
make pdf src=<archivo_a_compilar> M=<#columnas> N=<#filas>
```

Organización del directorio `src` (con datos del caso 3x3 y 3x4):
```
src
|-- syt_0.c
|-- makefile
|-- out2tex.sh
|-- obj
|   |-- syt_0.out
|-- pdf
|   |-- syt_0_3_3
|   |   |-- syt_0_3_3.pdf
|   |-- syt_0_3_4
|       |-- syt_0_3_4.pdf
|-- raw
|   |-- syt_0_3_3
|   |-- syt_0_3_4
|-- tex
    |-- syt_0_3_3.tex
    |-- syt_0_3_4.tex
```


### SYT_0: Tabla Cuadrada a Fuerza Bruta

El primer método consiste en partir de la configuración
inicial rellenando por filas de izquierda a derecha con
los números del `1` al `NM` donde `M` es la cantidad
de columnas y `N` es la cantidad de filas.

Ejemplo con `N=M=3`.

```
|1|2|3|
|4|5|6|
|7|8|9|
```

Cada celda consta de mínimos y máximos basales:

- **mínimos basales**: corresponde a la cantidad de celdas
que están a la izquierda y arriba de la celda (inclusive).
i.e. el área superior izquierda desde la celda.
- **máximos basales**: corresponde a la cantidad de celdas que
están a la derecha y abajo de la celda (inclusive) relativo
al número maximo `NM`.
i.e. `NM` menos el área inferior derecha.

El algoritmo consiste en partir de la configuración inicial
y devolverse. Al devolverse, se buscan los otros números
que podrían estar en esa celda. De no haber disponibles
se pasa al anterior y así. De haber un número disponible,
se pone ese en la celda y se avanza a la siguiente.

A modo de ejemplo:

```
|1|2|3|    |1|2|3|    |1|2|3|    |1|2|3|    |1|2|3|    |1|2|3|
|4|5|6| -> |4|5|6| -> |4|5|6| -> |4|5|7| -> |4|5|7| -> |4|5|7| 
|7|8|9|    |7|-|9|    |-|-|9|    |-|-|9|    |6|-|9|    |6|8|9|
```

Esto se repite hasta que no nos podamos seguir devolviendo.
