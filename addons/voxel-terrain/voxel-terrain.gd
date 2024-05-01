@tool
class_name VoxelTerrainPlugin extends EditorPlugin


var terrain_control: TerrainControl = null
var terrain_panel_button: Button = null

var terrain_marker: RID = RID()
var terrain_marker_mesh: Mesh = null

var editing: WeakRef = weakref(null)


func _enter_tree():
	terrain_control = preload("res://addons/voxel-terrain/terrain-control.tscn").instantiate()
	terrain_panel_button = add_control_to_bottom_panel(terrain_control, "Terrain Editing")
	
	create_terrain_marker()
	
	terrain_control.visibility_changed.connect(update_marker_visibility)
	
	terrain_panel_button.visible = false


func _exit_tree():
	remove_control_from_bottom_panel(terrain_control)
	terrain_control.queue_free()
	terrain_control = null
	
	RenderingServer.free_rid(terrain_marker)
	terrain_marker = RID()


func _handles(object):
	return object is VoxelTerrain


func _make_visible(visible):
	terrain_panel_button.visible = visible
	if visible:
		terrain_control.visible = terrain_panel_button.button_pressed
	else:
		terrain_control.visible = false


func _edit(object):
	editing = weakref(object)
	terrain_control.set_terrain(object)
	if is_instance_valid(object):
		# Make sure the editing marker is only visible when the terrain is, where the terrain is.
		RenderingServer.instance_set_scenario(terrain_marker, object.get_world_3d().scenario)
		RenderingServer.instance_set_visibility_parent(terrain_marker, object)


func _forward_3d_gui_input(viewport_camera: Camera3D, event: InputEvent):
	if terrain_control.visible:
		if event is InputEventMouse:
			# Update the target location with the current camera.
			var r0 = viewport_camera.project_ray_origin(event.position)
			var rd = viewport_camera.project_ray_normal(event.position)
			
			terrain_control.update_target_location(r0, rd)
			
			# Update the marker's transform, making sure the basis is the same as the terrain.
			RenderingServer.instance_set_transform(terrain_marker, Transform3D(editing.get_ref().transform.basis * terrain_control.get_size(), terrain_control.get_target_location()))
			
			if event.button_mask & MOUSE_BUTTON_MASK_LEFT:
				terrain_control.apply_edit()
				return EditorPlugin.AFTER_GUI_INPUT_STOP
		if event is InputEventMouseButton:
			# Use alt+scroll to update the size of the edit tool.
			if Input.is_key_pressed(KEY_ALT):
				if event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
					if event.pressed:
						terrain_control.update_size(terrain_control.get_size() - 1)
					return EditorPlugin.AFTER_GUI_INPUT_STOP
				if event.button_index == MOUSE_BUTTON_WHEEL_UP:
					if event.pressed:
						terrain_control.update_size(terrain_control.get_size() + 1)
					return EditorPlugin.AFTER_GUI_INPUT_STOP
			# Use ctrl+scroll to change the strength of the edit tool.
			if Input.is_key_pressed(KEY_CTRL):
				if event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
					if event.pressed:
						terrain_control.update_strength(terrain_control.get_strength() - 0.1)
					return EditorPlugin.AFTER_GUI_INPUT_STOP
				if event.button_index == MOUSE_BUTTON_WHEEL_UP:
					if event.pressed:
						terrain_control.update_strength(terrain_control.get_strength() + 0.1)
					return EditorPlugin.AFTER_GUI_INPUT_STOP
	return EditorPlugin.AFTER_GUI_INPUT_PASS


func create_terrain_marker():
	# This is just a simple RenderingServer terrain marker.
	terrain_marker_mesh = SphereMesh.new()
	terrain_marker_mesh.material = load("res://addons/voxel-terrain/materials/marker_material.tres")
	terrain_marker_mesh.radius = 1.0
	terrain_marker_mesh.height = 2.0
	
	terrain_marker = RenderingServer.instance_create()
	RenderingServer.instance_set_base(terrain_marker, terrain_marker_mesh)
	RenderingServer.instance_set_visible(terrain_marker, false)


func update_marker_visibility():
	RenderingServer.instance_set_visible(terrain_marker, terrain_control.is_visible_in_tree())
	print("Visible: " + str(terrain_control.is_visible_in_tree()))
