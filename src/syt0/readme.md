
### syt0: Fuerza Bruta (Bottom-Up)

#### Algoritmo

El método consiste en partir de la configuración
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

