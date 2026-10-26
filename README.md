# KramerLab 3x3 — versión de escritorio en C

Aplicación gráfica de escritorio para Linux desarrollada en **C y GTK 4**. Resuelve sistemas lineales de tres ecuaciones con tres incógnitas mediante la regla de Cramer y muestra las matrices usadas y los determinantes.

## Características

- Interfaz gráfica oscura con campos para los coeficientes y términos independientes.
- Botones para resolver, cargar un ejemplo y limpiar los campos.
- Cálculo de `Δ`, `Δx`, `Δy` y `Δz`.
- Muestra las matrices modificadas y las fórmulas `x = Δx/Δ`, `y = Δy/Δ`, `z = Δz/Δ`.
- Validación de entradas numéricas y detección de `Δ = 0`.

## Instalar dependencias en Garuda Linux / Arch Linux

```bash
sudo pacman -S --needed base-devel gtk4 pkgconf
```

## Compilar y ejecutar

Abre una terminal en la carpeta del proyecto y ejecuta:

```bash
make
./kramerlab-3x3
```

O compila y ejecuta en un solo paso:

```bash
make run
```

Para borrar el ejecutable:

```bash
make clean
```

## Ejemplo incluido

```text
 2x +  y -  z =   8
-3x -  y + 2z = -11
-2x +  y + 2z =  -3
```

La solución es `x = 2`, `y = 3`, `z = -1`.

## Nota matemática

Si `Δ = 0`, la regla de Cramer no determina una solución única. En ese caso, el sistema puede ser incompatible o tener infinitas soluciones; para distinguirlos se debe analizar el rango de las matrices.
