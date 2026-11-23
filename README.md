# 4ZT

<div align="center">
  <img src="showcase/cornellbox_2.jpg" alt="cornell box 2" width="300"/>
</div>

> `showcase/cornellbox_2.jpg` rendered with 4ZT v0.5.0

## Overview
4ZT is a physically based offline path tracer rendering engine. Its abstract rendering class allows for computation with multithreading CPU and GPU acceleration.

The engine is designed as a static API and is intended to be used as a library for rendering applications. As showcased by the demo `client` application as well as the `scout` real time renderer.

### Features
- Image (JPEG, PNG, PPM, HDR and EXR) and Video (MP4, MKV, GIF) outputs
- Multithreaded CPU and GPU compute renderers
- Full PBR materials (Metal, Dielectric, Lambertian)
- BVH acceleration structure
- HDR output

## Project Structure
Each project has a `cmake`, `src`, and `include` folders
This repository contains the following projects:
- `raytracer`: The core rendering engine library
- `client`: A simple demo application that renders the demo scene right away
- `scout`: A real time renderer that allows for scene manipulation and rendering

## Showcase
<img src="showcase/videodemo.gif" alt="cornell box 1" width="300"/>
<img src="showcase/cornellbox.jpg" alt="cornell box 1" width="300"/>

> Video demo (v0.5.0) and a rendered image of the Cornell Box scene (v 0.4.0).

## Scout Editor
This project is a work in progress. The Scout Editor is a real time renderer based on Vulkan 1.4 that allows for scene manipulation and rendering. It is built on top of the 4ZT engine and uses the same rendering pipeline.

It currently uses a simple dear imgui interface but I would like to implement something else like rmlui at some point. The editor is still in development and is not yet feature complete at all.

## License
This project is licensed under the MIT License. See `LICENSE` for more information.

## TODO Features
Pure Engine Features:
- Support for meshes
- Texture mapping
- Volumetric rendering
- Special materials: cloth, hair, skin, etc.

Scout Editor Features (WIP):
- Animation?
- Vertex manipulation


## Acknowledgements
Resources that helped a lot
- [Ray Tracing in One Weekend Series](https://raytracing.github.io/)
- [Physically Based Rendering:From Theory To Implementation](https://pbr-book.org/)