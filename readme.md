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

#### Compilar y ejecutar 

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

## Optimizaciones

#### [:heavy_check_mark:] Hilos y procesos 

Aquí se divide la tarea en productores y consumidores.

Los productores son hilos y se encargan de generar tablas.

Los consumidores son procesos y se encargan de resolver las tablas.

La sincronización se hace a través de colas. Cuando un consumidor
está listo se posiciona en la cola, mientras que cuando un productor
está listo saca "un consumidor" de la cola y le envía la información
necesaria para resolver la tabla.

#### [:x:] Preanálisis (implementando)

Aquí se usa el hecho de que las tablas que tienen subtablas cuyo
rango es una tabla mala, entonces la tabla grande es mala.

La idea es guardar los rangos malos en un archivo. Luego, al ejecutar
una tabla más grande se lee este archivo y se revisa si alguna subtabla
está presente.

Específicamente, las tablas se guardan en un árbol binario de búsqueda
(en este caso, un avl) de manera encadenada, es decir, cada tabla
tiene una "key" o "fingerprint" o un "hash" que se usa para posicionarle
en el árbol. Si el árbol ya tiene esa key, entonces se encadena a la lista
de tablas del nod que tiene la key. ie.
```
Nodo: 
    key
    tablas [lista de tablas que tienen la misma key]
```
