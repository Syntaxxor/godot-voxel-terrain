## An intermediary script for controlling and receiving UI events that interface
## with the TerrainControl GDExtension object.
@tool
extends TerrainControl


# Called when the node enters the scene tree for the first time.
func _ready():
	update_size(get_size())
	size_changed.connect(update_size)
	$PanelContainer/FlowContainer/StrengthSlider.value = get_strength()


func update_size(size: float):
	$PanelContainer/FlowContainer/SizeBox.value = size


func update_strength(value):
	set_strength(value)
	$PanelContainer/FlowContainer/StrengthSlider.value = get_strength()
