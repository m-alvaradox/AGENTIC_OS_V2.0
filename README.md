# AGENTIC OS V2.0

Proyecto en C que simula un sistema distribuido con un `launcher`, varias ventanas graficas y un servicio central llamado `IA Learner`. El `IA Learner` actua como data center: recibe texto desde ventanas remotas, forma oraciones, las clasifica con diccionarios y mantiene un perfil de usuario por sesion.

## Requisitos

En Linux/Ubuntu se necesita:

```bash
sudo apt install build-essential libx11-dev
```

Dependencias principales:

- `gcc`
- `make`
- `pthread`
- `libX11`

## Compilacion

Desde la raiz del proyecto:

```bash
make
```

Esto compila los tres modulos principales:

- `ialearner/ialearner`
- `launcher/launcher`
- `window/window`

Para recompilar todo desde cero:

```bash
make rebuild
```

Para limpiar archivos generados:

```bash
make clean
```

## Ejecucion

Primero se inicia el data center `IA Learner`. El parametro opcional `P` indica cuantas oraciones deben reunirse antes de activar los hilos de deteccion.

Terminal 1:

```bash
cd ialearner
./ialearner 2
```

Si no se envia `P`, se usa el valor por defecto definido en `include/config.h`. Si `P` supera la cantidad de CPUs disponibles, el servidor lo reduce automaticamente para evitar sobrecarga.

Luego se inicia el launcher.

Terminal 2:

```bash
cd launcher
./launcher
```

El launcher muestra un menu:

```text
1. Crear Window
2. Cerrar todas las ventanas
3. Mostrar procesos
4. Salir
```

## Uso Basico

Para una demostracion completa con textos de prueba y resultados esperados, consulte [`DEMO.md`](DEMO.md).

1. Ejecutar `ialearner`, opcionalmente con `P`.
2. Ejecutar `launcher`.
3. Elegir `1. Crear Window`.
4. Escribir texto en la ventana creada.
5. Presionar Enter para cerrar una oracion.
6. `IA Learner` encola la oracion.
7. Cuando hay `P` oraciones listas, el Loader despierta hasta `P` detectores del pool.
8. Cada detector clasifica una oracion en paralelo y vuelve a quedar suspendido.
9. Elegir `4. Salir` en el launcher para cerrar la sesion y ver el perfil final.

Si una ventana se cierra con texto pendiente, el servidor procesa ese ultimo fragmento antes de liberar el cliente.

## Arquitectura

El sistema usa una arquitectura cliente-servidor:

- `ialearner`: data center. Carga diccionarios, recibe conexiones, asigna puertos, administra sesiones, ejecuta Loader y detectores.
- `launcher`: proceso local de consola. Crea ventanas, cierra ventanas, muestra procesos y finaliza la sesion.
- `window`: proceso grafico creado por el launcher. Captura teclas con X11 y envia caracteres al data center.

Cada launcher tiene una sesion independiente dentro de `IA Learner`, con su propio perfil, cola de oraciones y ventanas asociadas.

## IPC

El IPC principal se implementa con sockets TCP:

- `launcher` se comunica con `ialearner` por el puerto de control `5001`.
- Cada `window` se conecta a un puerto asignado por `ialearner`, desde `5100` en adelante.

Aunque puede ejecutarse localmente con `127.0.0.1`, la estructura representa un escenario distribuido donde el data center puede estar en otra computadora.

## Concurrencia

El proyecto usa:

- procesos con `fork` y `execl` para crear ventanas;
- hilos `pthread` para atender launchers, ventanas, Loader y un pool permanente de detectores;
- mutex para proteger perfil, estado de sesion, puertos y procesamiento de lotes;
- condition variables para dormir el Loader y los detectores mientras no tengan trabajo.

Flujo concurrente:

```text
window -> caracteres -> IA Learner -> oracion -> cola
cola con P oraciones -> Loader -> despierta P detectores -> perfil
```

## Clasificacion

Cada oracion se clasifica con tecnica de bag of words. El texto se convierte a minusculas, se separa en palabras y se compara contra tres diccionarios:

- `diccionarios/correo.txt`
- `diccionarios/articulo.txt`
- `diccionarios/reporte.txt`

Si al menos tres palabras de un diccionario aparecen en la oracion, esa clase se considera candidata. Si hay mas de una clase candidata, se escoge la de mayor cantidad de coincidencias.

## Perfil De Usuario

El perfil se determina usando la Tabla 2 del enunciado:

```text
Correo                 -> Personal administrativo
Correo + Reporte       -> Personal tecnico
Correo + Articulo      -> Profesor
Articulo + Reporte     -> Estudiante
Correo + Articulo + Reporte -> No detectado
```

Cada detector actualiza el perfil de forma asincronica apenas termina de clasificar su oracion, sin esperar a que finalice el lote completo. Al final de la sesion se envia el ultimo perfil detectado al launcher.

## Mensajes De Consola

Durante la ejecucion, `IA Learner` imprime trazas utiles para verificar la concurrencia:

- `[COLA] Oracion recibida | pendientes=...`
- `[LOADER] Lote 01 iniciado...`
- `[LOTE 01][1/2] Texto: ...`
- `[LOTE 01][1/2] Clase: ... | coincidencias ... | usuario: ...`
- `[RESUMEN] Documentos procesados`

Estos mensajes ayudan a demostrar que el sistema no espera al cierre de la ventana, sino que procesa oraciones al presionar Enter.

## Fallo De IA Learner

Si `IA Learner` se cierra bruscamente, el launcher detecta que se perdio la comunicacion, cierra sus ventanas locales y finaliza. Esto evita dejar ventanas huerfanas sin conexion al servicio central.

## Archivos Generados

Durante la compilacion se pueden generar archivos como:

- `*.o`
- `*.d`
- `ialearner/ialearner`
- `launcher/launcher`
- `window/window`

Estos archivos no son codigo fuente. Para entregar el proyecto limpio, ejecutar:

```bash
make clean
```

Luego entregar la carpeta con los archivos `.c`, `.h`, `.txt`, `Makefile`, `README.md` y el PDF del reporte.
