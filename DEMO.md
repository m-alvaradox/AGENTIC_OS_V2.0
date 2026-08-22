# Demo de AGENTIC OS V2

Esta demostración comprueba el flujo completo usando dos terminales en la misma computadora.

## 1. Compilar

Desde la raíz del proyecto:

```bash
make rebuild
```

La compilación debe terminar sin errores.

## 2. Iniciar IALearner con P=2

En la primera terminal:

```bash
cd ialearner
./ialearner 2
```

Debe aparecer:

```text
IA LEARNER - DATA CENTER
[CONFIG] Detectores (P)       : 2
[ESTADO] Servidor listo; esperando un launcher.
```

## 3. Iniciar el launcher

En la segunda terminal:

```bash
cd launcher
./launcher
```

## 4. Prueba principal: detectar Profesor

1. En el launcher, selecciona `1. Crear Window` dos veces.
2. En la primera ventana escribe esta oración y presiona Enter:

```text
Thank Please Regards Meeting
```

3. IALearner debe indicar `Pendientes: 1/2`. Todavía no debe procesar el lote.
4. En la segunda ventana escribe esta oración y presiona Enter:

```text
Analysis Results Method Study
```

5. Al completar las dos oraciones, IALearner debe mostrar mensajes equivalentes a:

```text
[LOADER] Lote 01 iniciado: despertando 2 detector(es).
[LOTE 01][1/2] Clase: Correo electronico  | coincidencias C:4 A:0 R:0
[LOTE 01][2/2] Clase: Articulo cientifico | coincidencias C:0 A:4 R:0 | usuario: Profesor
[LOADER] Lote 01 completado.
```

El orden de los mensajes de los dos detectores puede variar porque trabajan concurrentemente. Eso es normal.

6. En el launcher selecciona `3. Mostrar procesos` para comprobar que monitorea las ventanas locales.
7. Selecciona `4. Salir`.
8. El launcher debe mostrar como resultado final:

```text
Profesor
```

## 5. Prueba de exceso de oraciones: 2 + 2 + 1

Reinicia IALearner y el launcher para obtener una sesión limpia. Conserva `P=2` y crea una ventana.

Escribe cinco veces la siguiente oración, presionando Enter después de cada línea:

```text
Thank Please Regards Meeting
Thank Please Regards Meeting
Thank Please Regards Meeting
Thank Please Regards Meeting
Thank Please Regards Meeting
```

El comportamiento esperado es:

- Las primeras dos oraciones forman el primer lote.
- Las siguientes dos forman el segundo lote.
- La quinta permanece pendiente porque no completa otro lote de dos.
- Al seleccionar `4. Salir`, se procesa el remanente de una oración.
- Las estadísticas finales muestran cinco correos.
- El launcher muestra `Personal Administrativo`.

En la terminal de IALearner deben aparecer dos mensajes de lote con dos oraciones y uno final con una oración:

```text
[LOADER] Preparando lote | oraciones=2 | limite P=2
[LOADER] Preparando lote | oraciones=2 | limite P=2
[LOADER] Preparando lote | oraciones=1 | limite P=2
```

## 6. Textos para probar los cuatro usuarios

Reinicia ambos programas antes de cada caso para no acumular clases de una prueba anterior. Con `P=2`, ingresa las dos oraciones indicadas y presiona Enter después de cada una.

### Personal administrativo

```text
Thank Please Regards Meeting
Attached Information Update Schedule
```

Resultado esperado: `Personal Administrativo`.

### Personal técnico

```text
Thank Please Regards Meeting
Network Security Server Performance
```

Resultado esperado: `Personal Técnico`.

### Profesor

```text
Thank Please Regards Meeting
Analysis Results Method Study
```

Resultado esperado: `Profesor`.

### Estudiante

```text
Analysis Results Method Study
Network Security Server Performance
```

Resultado esperado: `Estudiante`.

## 7. Checklist visual

Durante la demostración comprueba lo siguiente:

- [ ] El proyecto compila sin errores.
- [ ] El launcher crea dos ventanas independientes.
- [ ] La opción `Mostrar procesos` presenta PID y estado de cada ventana.
- [ ] Una sola oración queda esperando cuando `P=2`.
- [ ] Al llegar la segunda oración, el Loader despierta los detectores.
- [ ] Las letras son recibidas por IALearner desde las ventanas.
- [ ] Cada oración se clasifica con al menos tres palabras del diccionario.
- [ ] El perfil cambia apenas termina cada detector.
- [ ] Las oraciones excedentes esperan el siguiente lote.
- [ ] El remanente inferior a P se procesa al cerrar.
- [ ] El launcher recibe y muestra el tipo de usuario final.
- [ ] Al salir no quedan ventanas ni procesos zombis.

## Nota

No escribas una tercera clase documental en la misma sesión cuando quieras demostrar uno de los cuatro perfiles. La tabla del proyecto no define una categoría para la combinación simultánea de correo, artículo y reporte, por lo que el resultado sería `No detectado`.
