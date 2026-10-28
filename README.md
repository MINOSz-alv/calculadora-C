# KramerLab — versión de escritorio en C

Aplicación gráfica de escritorio para Linux desarrollada en **C y GTK 4**. Permite elegir y resolver sistemas lineales de **2 ecuaciones con 2 incógnitas (2x2)** o de **3 ecuaciones con 3 incógnitas (3x3)** mediante la regla de Cramer. Muestra las matrices usadas, los determinantes y el procedimiento.

## Características

- Interfaz gráfica oscura con campos para los coeficientes y términos independientes.
- Selector del tamaño del sistema: 2x2 o 3x3.
- Botones para resolver, cargar un ejemplo y limpiar los campos.
- Cálculo de `Δ` y de los determinantes correspondientes a cada incógnita.
- Muestra las matrices modificadas y la fórmula de Cramer para cada incógnita del sistema seleccionado.
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

## Compilación para Windows

El workflow de GitHub Actions `.github/workflows/windows-installer.yml` compila la aplicación en Windows con GTK 4 y publica `kramerlab-3x3.exe` como artefacto descargable. Se ejecuta al hacer push o pull request, y también puede iniciarse manualmente desde la pestaña **Actions** de GitHub.

El artefacto contiene el ejecutable; para ejecutarlo en Windows también se requiere el runtime de GTK 4.

## Ejemplos incluidos

Sistema 2x2 (solución: `x = 2`, `y = 3`):

```text
 x +  y = 5
2x -  y = 1
```

Sistema 3x3 (solución: `x = 2`, `y = 3`, `z = -1`):

```text
 2x +  y -  z =   8
-3x -  y + 2z = -11
-2x +  y + 2z =  -3
```

La solución es `x = 2`, `y = 3`, `z = -1`.

## Nota matemática

Si `Δ = 0`, la regla de Cramer no determina una solución única. En ese caso, el sistema puede ser incompatible o tener infinitas soluciones; para distinguirlos se debe analizar el rango de las matrices.
