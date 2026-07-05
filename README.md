# AGENTIC OS V2.0

Proyecto en C que simula un sistema con un launcher, ventanas cliente y un servicio central llamado IA Learner. El IA Learner funciona como data center: recibe documentos desde las ventanas, los clasifica usando diccionarios y genera un perfil de usuario por sesion.

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

Primero se inicia el data center IA Learner.

Terminal 1:

```bash
cd ialearner
./ialearner
```

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

## Uso basico

1. Ejecutar `ialearner`.
2. Ejecutar `launcher`.
3. Elegir `1. Crear Window`.
4. Escribir texto en la ventana creada.
5. Cerrar la ventana con `Esc` o con el boton de cerrar.
6. IA Learner clasifica el documento recibido.
7. Elegir `4. Salir` en el launcher para cerrar la sesion y ver el perfil.

## Multi-launcher

IA Learner puede aceptar varios launchers conectados al mismo tiempo. Cada launcher tiene su propia sesion, su propio perfil y sus propias ventanas.

Ejemplo:

```text
Launcher 1 -> sesion 1 -> perfil 1
Launcher 2 -> sesion 2 -> perfil 2
```

Los puertos de las ventanas son asignados por IA Learner para evitar conflictos entre launchers.

## Fallo de IA Learner

Si IA Learner se cierra bruscamente, el launcher detecta que se perdio la comunicacion, cierra sus ventanas locales y finaliza. Esto evita dejar ventanas huerfanas sin conexion al servicio central.

## Archivos generados

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

Luego entregar la carpeta con los archivos `.c`, `.h`, `.txt`, `Makefile` y este `README.md`.

## Diccionarios

Los documentos se clasifican usando los archivos:

- `diccionarios/correo.txt`
- `diccionarios/articulo.txt`
- `diccionarios/reporte.txt`

Cada documento se compara contra estos diccionarios para determinar si corresponde a correo, articulo o reporte.
