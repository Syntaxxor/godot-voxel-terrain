## An intermediary script for controlling and receiving UI events that interface
## with the TerrainControl GDExtension object.
@tool
extends TerrainControl


@onready var size_box = $PanelContainer/FlowContainer/SizeBox
@onready var strength_slider = $PanelContainer/FlowContainer/StrengthSlider


# Called when the node enters the scene tree for the first time.
func _ready():
	update_size(get_size())
	update_strength(get_strength())


func update_size(size: float):
	set_size(size)
	size_box.value = get_size()


func update_strength(value):
	set_strength(value)
	strength_slider.value = get_strength()
