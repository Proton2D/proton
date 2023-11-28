# proton

## About
<b>Proton2D</b> is a simple open-source game engine specifically designed to make 2D platformer games. 

The main features of the engine are:
- Game Editor written using <a href="https://github.com/ocornut/imgui">ImGui</a> library,
- Entity Component System (ECS) architecture powered by the <a href="https://github.com/skypjack/entt">EnTT</a> library,
- External Physics Engine <a href="https://github.com/erincatto/box2d">Box2D</a>,
- Prebuilt Entity Components,
- Scenes system to manage game entities,
- Native C++ Entity Scripting,
- Spritesheet support,
- Spritesheet Tile-based Animation,
- Resizable Sprites using the 9-scaling method,
- Entity Prefabs (will be reworked).

<details>
<summary><b>Game Editor Preview (expand)</b></summary>
<img src="https://i.imgur.com/jJWpWKr.png" alt="Game editor"></img>
</details>

## Getting Started
At the moment, the Proton engine is compatible with Windows only. Linux support is planned to be added in the near future.

#### Cloning The Repository
Proton uses the <b>git submodules</b>, therefore repository cloning should be done with the following command:
```
git clone --recursive https://github.com/Proton2D/proton
```
If you happen to clone this repository non-recursively, use `git submodule update --init ` to clone the necessary submodules.

## Building
<b>Build configuration tool:</b> Premake 5

<b>Supported platforms:</b> Windows

### Building Solution Projects
Run the ```Win-Build-VS22``` script to generate solution files for <b>Microsoft Visual Studio 2022</b> via <b>Premake5</b>.

### Building The Game
Run the ```Win-Build-Game``` script to build and copy the game executable and content from the project directory to a separate output build directory. You can choose a project by providing its name, the configuration of the build, and the target output directory. Default values can be modified inside the script. Proton does not currently support binary asset packing, all files from the `content` directory will be copied directly to the output directory while running the script.

### Build Configurations
There are three types of build configurations in Proton2D:
- <b>Debug configuration</b>: This build is intended for development and debugging purposes.
- <b>Release configuration</b> This build offers more optimized performance than the debug build, while still retaining the full functionality of the game editor.
- <b>Distribution configuration</b>: This is the deployment-ready build, optimized for end users. It excludes a game editor, ensuring an efficient application.

## The Game Engine Architecture
### Inspiration
The Proton2D game engine architecture is mainly based on the 
<a href="https://github.com/TheCherno/Hazel">Hazel Engine</a> architecture. The resources and materials provided there by the Cherno helped me learn a lot about the game engine and software architecture, which I am very thankful for. I highly recommend checking it out for anyone who is interested in the game engine architecture. Note: I am not an expert game engine developer, this is just my personal project that I worked on, and I will continue working on for some time.

Modules as 
<b>Renderer</b>,
<b>Event</b> system,
<b>Debug</b> utilities and implementation for Windows
<b>Input</b> and application 
<b>Window</b> were directly copied from the Hazel source code with some slight modifications. The organization of code into modules, represented by the `src/Proton` directory structure, closely resembles the structure found in Hazel and other game engines.
Most of the libraries used in Proton are also used in Hazel.
The only difference is the library for entity serialization, which Proton happens to use, is the <a href="https://github.com/nlohmann/json">nlohmann/json</a> library.

### Libraries Used
- <a href="https://github.com/glfw/glfw">GLFW</a>
- <a href="https://glad.dav1d.de/">Glad</a>
- <a href="https://github.com/nothings/stb/blob/master/stb_image.h">stb_image</a>
- <a href="https://github.com/g-truc/glm">glm</a>
- <a href="https://github.com/ocornut/imgui">ImGui</a>
- <a href="https://github.com/skypjack/entt">EnTT</a>
- <a href="https://github.com/erincatto/box2d">Box2D</a>
- <a href="https://github.com/nlohmann/json">nlohmann/json</a>
- <a href="https://github.com/gabime/spdlog">spdlog</a>

### Game Engine Modules
The `src/Proton` directory was divided into the following modules.
| Module | Short description |
| ----------- | ---------- |
| <b>Core</b> | Core engine components like the `Application` class, `Window`, and `Input` interface. |
| <b>Debug</b> | Spdlog logger wrapped with macros. Better assertions that use debugbreak. An `Instrumetor` class for measuring code performance. |
| <b>Events</b> | `Event` system for handling events with the `EventDispatcher` class. The module contains window, key, and mouse event classes. |
| <b>Graphics</b> | The module features a 2D Batch Renderer from Hazel engine. Additional classes are: `Sprite`, `Spritesheet`, `ResizableSprite`, `SpriteAnimation`, `Camera`. |
| <b>Scene</b> | Scene module contains ECS integration consisting of `Scene` class which manages entities on the screen and owns `entt::registry`, an `Entity` class wrapper which provides several methods to make changes to components associated with an `entt::entity`. The `Components.h` file contains all the available components that can be added to an entity. This module also contains utility classes like `SceneManager` to manage game scenes and `PrefabManager` (will be reworked) to create entity prefabs. |
| <b>Physics</b> | Integration of the Box2D physics engine. The components for entities are `RigidbodyComponent` and `BoxColliderComponent`.  |
| <b>Scripting</b> | Contains an `EntityScript` base class and the `ScriptFactory` for keeping track of created `EntityScript` derived classes and creating appropriate instances by providing script class name as a string. Scripts are at the moment written natively in C++. |
| <b>Assets</b> | Contains very basic `AssetManager` class to manage game resources and a `SceneSerializer` class to serialize and deserialize scenes and entities. |
| <b>Editor</b> | Game editor interface implementation using the ImGui library. |

### Game Engine Editor
The Proton2D game engine editor consists of several panels, each having it's own unique role in the game development process.
| Panel | Short description |
| ----------- | ---------- |
| <b>Scene Hierarchy</b> |  Manages the scene's entity hierarchy structure. Right-click to add a new entity at the scene root, or click on an entity to add a child. Entities can be reorganized through drag-and-drop.  |
| <b>Inspector</b> | Panel where you can edit entities by modifing their component values. |
| <b>Scene</b> | Scene simulation control buttons: Play, Pause, Resume, and Stop. The view of scenes loaded in memory. This will be changed to scene tabs above the editor in the future. |
| <b>Viewport</b> | Scene viewport that displays a rendered game view. You can move the editor camera by holding the right mouse button, and select and move entities using the left mouse button. Camera zoom can be changed by using the scroll wheel. |
| <b>Prefab</b> | List of prefabs that can be spawned or deleted. |
| <b>Misc</b> | General application settings and statistics. |

The editor is integrated into the game runtime and not built as a separate application, because the engine does not have external language scripting nor hot reloading implemented yet.

### Entity Scripting
Proton at the moment, offers only Native C++ Scripting. To create an entity script, you must create a class that derives from the `EntityScript` base class. Inside the created class, a macro `ENTITY_SCRIPT_CLASS(class)` must be placed (under the public members). It will register a script inside the `ScriptFactory` class.

The approach of native scripting has the disadvantage of requiring a recompilation and restart of the application for script changes, as it cannot hot-reload scripts during runtime. Native C++ scripting however, is generally faster in terms of performance than external language scripting. The external scripting engine will probably be added in the future. It was not implemented yet due to the lack of time.

## Sandbox Project
The `sandbox` is the project in which the game is developed. It contains an example game made in Proton2D. Window properties can be changed by modifing the `sandbox/app-config.json` config file. A proper script for creating new game projects from the template will be added soon.

## The Plans
The current goal of the project is to have networking implemented and a basic game UI system. Additional features and enhancements to the editor will be implemented as well. The work will be primarily done in a private branch. This branch will also receive updates from time to time.

## License
&copy; Licensed under the MIT License.
