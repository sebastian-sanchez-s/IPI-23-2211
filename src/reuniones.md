# Viernes 14 de Julio

- Un manual online 
https://people.inf.ethz.ch/fukudak/soft/soft.html

- Uno de los primeros algoritmos para pasar de V- H- (esto es como lo mas basico de geometria poliedral) lo implemento Avis
http://cgm.cs.mcgill.ca/~avis/C/lrs.html

- Y ahi tiene links a otras libs relacionadas 
http://cgm.cs.mcgill.ca/~avis/C/lrslib/links.html

Hora de reuiones por zoom: **Viernes 2 pm Hora Chile**

# Viernes 21 de Julio

- Revisar el libro de Matousek *Understanding and Using linear programming*

- Con los tableaus que ya hay computados testear que no contengan los prohibidos de Mallows - Vanderbei

- En general para chequear si es outer sum o no hay que chequear desigualdades
    
## Ejemplo:
Tableu = 1 | 2 | 5
         3 | 4 | 6
 
para saber si es suma exterior o no, hay que revisa si existen numeros reales c1,c2,c3 (uno por columna) y 
f1,f2 (uno por fila) tales que
c1 < c2 < c3
f1 < f2

c1+f1 < c2+f1 < c1+f2 < c2+f2 < c3+f1 < c3+f2
Este orden esta dado precisamente por el orden en el que aparecen los numeros en el tableau
Algunas de estas son redundantes! pero para que sea mas uniforme las ponemos todas.

*OJO*: las desigualdades deben ser estrictas, si fueran simplemente menor o igual se podria poner todo igual a cero.


 
