# YelaGL — Renderizado 3D de Modelos GLB y Trayectoria Bézier en OpenGL

Proyecto de computación gráfica desarrollado en **C++20** y **OpenGL 3.3 Core Profile** que implementa la carga de modelos 3D en formato binario glTF 2.0 (`.glb`), gestión modular de *shaders* GLSL, texturizado estilo retro PSX y animación de trayectoria parabólica mediante una **Curva de Bézier Cúbica**.

**Autor:** Camilo Yela

---

## Características Principales

* **Carga de Modelos `.glb`:** Lectura e interpretación de mallas, índices, coordenadas UV y texturas incrustadas utilizando `fastgltf` y `stb_image`.
* **Estética Retro (PSX):** Filtrado de texturas mediante `GL_NEAREST` para conservar la nitidez pixelada característica de los modelos *low-poly* de primera generación 3D.
* **Arquitectura Modular de Shaders:** Encapsulamiento de compilación, enlazado y envío de variables *uniform* (`uMVP`, `texture1`) en una clase `Shader` reutilizable.
* **Animación por Curva de Bézier Cúbica:** Simulación continua de lanzamiento parabólico definida por 4 puntos de control (P₀, P₁, P₂, P₃) evaluados en tiempo real.
* **Cámara Interactiva:** Navegación libre en el espacio 3D independiente de la tasa de fotogramas (`deltaTime`).

---

## Tecnologías y Librerías Utilizadas

* **Lenguaje:** C++20
* **API Gráfica:** OpenGL 3.3 (Core Profile)
* **Gestor de Paquetes:** `vcpkg` (Modo Manifiesto con `vcpkg.json`)
* **Dependencias:**
  * [GLFW](https://glfw.org) — Creación de ventana, contexto OpenGL y gestión de entradas de teclado.
  * [GLAD](https://dav1d.de) — Cargador de punteros de funciones modernas de OpenGL.
  * [GLM](https://github.com) — Operaciones matemáticas de vectores y matrices 3D (`Model`, `View`, `Projection`).
  * [fastgltf](https://github.com) — Parser de alto rendimiento para activos glTF 2.0 / `.glb`.
  * [stb_image](https://github.com) — Decodificación de texturas incrustadas desde memoria.

---

## Estructura del Proyecto

```text
YelaGL/
├── Main.cpp                    # Bucle principal, cámara, lógica Bézier y carga de mallas
├── shader.h                    # Declaración de la clase Shader
├── shader.cpp                  # Implementación de lectura, compilación y enlace de shaders
├── stb.cpp                     # Implementación de stb_image (STB_IMAGE_IMPLEMENTATION)
├── default.vert                # Vertex Shader (transformación espacial con matriz MVP)
├── default.frag                # Fragment Shader (muestreo de textura 2D)
├── psx_hatchet_low-poly.glb    # Modelo 3D binario con geometría y textura incrustada
├── vcpkg.json                  # Archivo de manifiesto de dependencias
└── README.md                   # Documentación del proyecto
```

---

## Controles

| Tecla | Acción |
| :--- | :--- |
| **`W`** / **`W`** | Acercar la cámara hacia el frente |
| **`S`** / **`S`** | Alejar la cámara hacia atrás |
| **`A`** / **`A`** | Desplazar la cámara a la izquierda |
| **`D`** / **`D`** | Desplazar la cámara a la derecha |
| **`R`** | Reiniciar el lanzamiento desde el origen ($P_0$) |
| **`ESC`** | Cerrar la aplicación |

---

## Fundamento Matemático: Curva de Bézier Cúbica

La trayectoria de lanzamiento del objeto se calcula en cada fotograma evaluando el polinomio de Bernstein de grado 3 para un parámetro normalizado $t \in [0, 1]$:

$$B(t) = (1 - t)^3 P_0 + 3(1 - t)^2 t P_1 + 3(1 - t) t^2 P_2 + t^3 P_3$$

* **$P_0$:** Punto inicial de lanzamiento.
* **$P_1, P_2$:** Puntos de control que moldean la elevación y curvatura del arco.
* **$P_3$:** Punto final de impacto (objetivo).

---

## Requisitos y Compilación en Visual Studio

1. **Requisitos previos:**
   * Visual Studio 2022 o superior con la carga de trabajo **Desarrollo para el escritorio con C++**.
   * Integración de **vcpkg** habilitada en modo manifiesto.

2. **Configuración del proyecto:**
   * Verificar que **Propiedades > General > Estándar del lenguaje C++** esté configurado en **ISO C++20 (`/std:c++20`)**.
   * Verificar que **Propiedades > vcpkg > Use Vcpkg Manifest** esté en **`Yes`**.
   * Asegurarse de que los archivos `default.vert`, `default.frag` y `psx_hatchet_low-poly.glb` estén ubicados en el directorio de trabajo del proyecto (junto a `Main.cpp`).

3. **Ejecución:**
   * Abrir la solución `.sln` en Visual Studio.
   * Compilar y ejecutar presionando **`F5`** (la primera compilación descargará automáticamente las dependencias declaradas en `vcpkg.json`).
