

<img width="1435" height="1063" alt="image-21-1" src="https://github.com/user-attachments/assets/247292ff-03b5-432e-9d3c-738ce2588063" />

Gravitas Runtime

Gravitas Runtime is a lightweight, performance-focused engine designed for real-time rendering and interaction on constrained hardware platforms such as the PSP. It serves as the execution layer for scenes authored in Gravitas Studio, with a strong emphasis on efficiency, determinism, and low-level control.

Features

Static Mesh Rendering
Efficient rendering of preprocessed geometry with support for batching and minimal state changes.

Render Cache System
A core rendering layer that builds and organizes draw data ahead of time. Geometry is grouped and cached to eliminate redundant per-frame work, keeping CPU cost low and frame times stable.

Textured Quads
Fast quad rendering for UI, sprites, and simple world elements, integrated directly into the main pipeline.

Controller Input System
Responsive input handling supporting both event-driven and continuous input modes.

Camera System
Flexible camera controls for movement, rotation, and scene navigation.

Messaging System
Lightweight message-based architecture enabling decoupled communication between systems and logic units.

Built-in Profiling
Integrated profiling tools allow real-time measurement of system performance, making it easy to identify bottlenecks and validate optimizations directly on hardware.

Font-Based Debug Logging
On-screen logging using the runtime font renderer enables immediate visual feedback without relying on external debug consoles. Designed for constrained environments, this system allows continuous debugging during active rendering.

Design Philosophy

Gravitas Runtime is built around a few core principles:

Performance First
Minimize per-frame work. Prepare and cache data up front whenever possible.
Deterministic Behavior
Systems behave predictably, making debugging and optimization straightforward.
Explicit Data Flow
Clear data ownership and minimal abstraction keep the engine easy to reason about.
Modular Architecture
Systems remain independent and communicate through messaging.
Hardware-Aware Design
Every decision is made with platform constraints in mind, especially CPU cost and memory bandwidth.
Pipeline
Scenes are authored in Gravitas Studio
Data is exported into a custom chunk-based binary format
Gravitas Runtime loads the scene, builds render caches, and executes directly on target hardware
Current Status

The runtime provides a solid foundation for rendering and interaction, with ongoing work focused on:

Core gameplay systems
Physics and collision systems
Expanded rendering features
Streaming and large-scale environments
