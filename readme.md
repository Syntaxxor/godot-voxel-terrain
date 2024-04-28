# Godot Voxel Terrain

This is a GDExtension plugin for Godot that adds voxel terrain. The terrain can be edited in the editor and properly saves and loads.

## Features
- In-editor modification.
- Saving and loading.
- Mesh generation.
- Collision.
- A chunk system that removes limits on terrain size and shape.

### Not implemented:
- Multiple surfaces
- LOD
- Multiple editing tools and shapes

## To use:
Create a VoxelTerrain object. When it is selected, a "Terrain Editing" tab will appear at the bottom of the screen. Click on that tab to open the editor.

Left-click in the world to use your current tool, as marked by the red sphere.

The strength slider goes from -1.0 to 1.0, from left to right.

Currently, there is only a terrain sculpting tool. This creates or destroys terrain based on the tool strength.

Hotkeys:
- Ctrl+Scroll to change tool strength.
- Alt+Scroll to change tool size.

## Downloading:
To get a pre-built release, go to the "Releases" section on the right side of the GitHub page. Download voxel-terrain.zip and extract "addons/voxel-terrain" into your own project's addons folder.

## Building:
To build, you will need to acquire scons as indicated in the Godot [GDExtension C++ example](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html) and [Godot Engine Compiling](https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html#toc-devel-compiling) tutorials.

Once you have, simply run ``scons target=template_release`` in the same folder as `project.godot`. You should now be able to copy the addons/voxel-terrain folder into your own project's addons folder.

## Note:
This addon is exclusively for terrain editing. If you'd like to use things like foliage, I recommend [HungryProton's Scatter Plugin](https://github.com/HungryProton/scatter).