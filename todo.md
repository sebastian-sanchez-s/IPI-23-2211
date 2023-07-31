Ruta para optimizar:
- Distribuir mejor los hilos y buscar una forma de reusarlos i.e. hacer una thread pool.
- Calcular más cosas de antemano (sencillo) i.e. tener tablas en memoria desde el inicio en vez de pedirlas con malloc/calloc.
- Implementar un patrón productor-consumidor. Unos hilos producen tablas y otros las revisan. El plan es hacer 
una queue de tamaño fijo donde los productores vayan poniendo las tablas.

Conflictos:
- cddlib arma las matrices por columnas, pidiendo memoria para cada una (y la matriz misma). Esto dificulta el reuso.
