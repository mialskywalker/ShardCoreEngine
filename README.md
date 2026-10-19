# ShardCore Engine

**A custom C++ game engine built from scratch using Direct3D 12.**

ShardCore is an ongoing personal project focused on exploring low-level game engine development, graphics programming, and engine architecture.

The goal is to build a modular foundation for rendering and gameplay systems while gaining a deeper understanding of how modern game engines work internally.

## Features

ShardCore currently focuses on building its core engine foundation, including:

- **Direct3D 12 Rendering** – Low-level graphics rendering using Microsoft's Direct3D 12 API.
- **Graphics Pipeline** – Pipeline state objects, root signatures, shaders, and GPU resource management.
- **Rendering Resources** – Vertex buffers, index buffers, and GPU resource handling.
- **3D Transformations** – World, view, and projection transformations.
- **Depth Buffering** – Depth testing for rendering 3D geometry.
- **Input Handling** – Basic input support.
- **Engine Architecture** – Separation between the core engine library and a Sandbox application for development and testing.

## Technologies

- **Language:** C++ 
- **Graphics API:** Direct3D 12
- **Shaders:** HLSL
- **Platform:** Windows
- **Development Environment:** Visual Studio 2022

## Project Structure

```text
ShardCore/
├── Engine/       # Core engine library
├── Sandbox/      # Development and testing application
└── External/     # Third-party dependencies
```

The engine is developed as a reusable library, while the Sandbox application serves as an environment for testing rendering features and engine systems.

## Development Status

**ShardCore is currently in active development.**

The project is being built incrementally, starting with the rendering foundation and gradually expanding toward a more complete game engine.

Planned areas of development include:

- Texture loading and rendering
- 3D model loading
- Scene and entity management
- Camera and transformation systems
- Debug visualization tools
- Gameplay systems
- Editor tooling

These features are part of the development roadmap and may change as the project evolves.

## Development Journey

The development process is documented through a YouTube devlog series covering the implementation of engine features, technical challenges, and progress.

**YouTube:** https://www.youtube.com/playlist?list=PLE9YI7gKzCSc

## About

ShardCore is primarily an educational and portfolio project, with the long-term goal of becoming a foundation for developing games and experimenting with gameplay systems.

The project is developed independently as part of my ongoing journey in game engine and graphics programming.

---

*Built with C++ and Direct3D 12.*
