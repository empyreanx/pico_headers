pico_headers
--------------------------------------------------------------------------------
A collection of cross-platform single header libraries written in C.


Library | Docs | Description | Version
------- | -----| ------------| -------
**[pico_b64](pico_b64.h)**   | **[docs](https://empyreanx.github.io/docs/ph/pico__b64_8h.html)**  | Base64 encoding/decoding library             | 0.1
**[pico_ecs](pico_ecs.h)**   | **[docs](https://empyreanx.github.io/docs/ph/pico__ecs_8h.html)**  | Pure and simple ECS                          | 2.3
**[pico_gfx](pico_gfx.h)**   | **[docs](https://empyreanx.github.io/docs/ph/pico__gfx_8h.html)**  | Graphics library based on sokol_gfx          | 0.1
**[pico_gl](pico_gl.h)**     | **[docs](https://empyreanx.github.io/docs/ph/pico__gl_8h.html)**   | Graphics library based on OpenGL             | 0.1
**[pico_hit](pico_hit.h)**   | **[docs](https://empyreanx.github.io/docs/ph/pico__hit_8h.html)**  | 2D collision detection (SAT) and ray casting | 0.2
**[pico_log](pico_log.h)**   | **[docs](https://empyreanx.github.io/docs/ph/pico__log_8h.html)**  | Minimal and flexible logging framework       | 1.0
**[pico_math](pico_math.h)** | **[docs](https://empyreanx.github.io/docs/ph/pico__math_8h.html)** | 2D math library for games                    | 2.0
**[pico_qt](pico_qt.h)**     | **[docs](https://empyreanx.github.io/docs/ph/pico__qt_8h.html)**   | A simple quadtree library                    | 1.1
**[pico_time](pico_time.h)** | **[docs](https://empyreanx.github.io/docs/ph/pico__time_8h.html)** | Simple time management library               | 0.1
**[pico_unit](pico_unit.h)** | **[docs](https://empyreanx.github.io/docs/ph/pico__unit_8h.html)** | Bare-bones unit testing framework            | 1.1

These libraries are as-is, however, suggestions for improvements or bug fixes are appreciated. Please raise an issue before submitting a PR with new features. Bug fixes are always welcome!

The API of libraries with versions less than 1.0 are subject to changes without warning. These changes might crash your code or cause other problems. Libraries with versions equal to or greater than 1.0 are more stable and will generally only be changed when adding features or making bug fixes. Changes may still break the API, but will be limited in scope and should not introduce unpredictable behavior at runtime.

The examples and tests compile and run on Linux (GCC), Windows (MSYS2/MinGW64), and MacOS (Clang). The only exception is the Rogue demo, which only compiles/runs on Linux and MacOS.

Most libraries are licensed under your choice of zlib or the public domain. The remaining libraries are licensed under the MIT license.

Deprecated libraries (pico_sat and the old pico_math) can be found [here](https://github.com/empyreanx/pico_headers_deprecated)

I give my thanks to [Randy Gaul](https://github.com/RandyGaul) for inspiration, answering my questions, and the template for this project.
