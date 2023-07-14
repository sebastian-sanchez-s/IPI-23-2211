# Contexto

Este repositorio contiene código generado
en la investigación de invierno IPI-23-2211

# Standard Young Tableux (SYT)

Una tabla de Young en formato estándar consiste en
un arreglo 2-dimensional de números (una matriz, aunque
no necesariamente cuadradada.) 
de tal forma que cada fila y cada columna forme un
arreglo creciente.

### SYT_0: Tabla Cuadrada a Fuerza Bruta

El primer método consiste en partir de la configuración
inicial rellenando por filas de izquierda a derecha con
los números del `1` al `N^2` como se ilustra acontinuación
con `N=3`.

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
están a la derecha y abajo de la celda (inclusive).
i.e. `N^2` menos el área inferior derecha.

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
