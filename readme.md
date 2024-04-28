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