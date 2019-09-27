/*************************************************************************/
/*   control_editor_plugin.cpp                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "control_editor_plugin.h"

#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/multi_node_edit.h"
#include "scene/gui/box_container.h"
#include "scene/gui/menu_button.h"

Vector2 AnchorAndMarginsEditor::_position_to_anchor(const Control *p_control, Vector2 position) {
	ERR_FAIL_COND_V(!p_control, Vector2());

	Rect2 parent_rect = p_control->get_parent_anchorable_rect();
	ERR_FAIL_COND_V(parent_rect.size.x == 0, Vector2());
	ERR_FAIL_COND_V(parent_rect.size.y == 0, Vector2());

	return (p_control->get_transform().xform(position) - parent_rect.position) / parent_rect.size;
}

void AnchorAndMarginsEditor::_set_anchors_and_margins_preset(Control::LayoutPreset p_preset) {
	EditorNode::get_undo_redo()->create_action(TTR("Change Anchors and Margins"));

	for (List<Control *>::Element *E = controls.front(); E; E = E->next()) {
		Control *control = E->get();
		EditorNode::get_undo_redo()->add_do_method(control, "set_anchors_preset", p_preset);
		switch (p_preset) {
			case PRESET_TOP_LEFT:
			case PRESET_TOP_RIGHT:
			case PRESET_BOTTOM_LEFT:
			case PRESET_BOTTOM_RIGHT:
			case PRESET_CENTER_LEFT:
			case PRESET_CENTER_TOP:
			case PRESET_CENTER_RIGHT:
			case PRESET_CENTER_BOTTOM:
			case PRESET_CENTER:
				EditorNode::get_undo_redo()->add_do_method(control, "set_margins_preset", p_preset, Control::PRESET_MODE_KEEP_SIZE);
				break;
			case PRESET_LEFT_WIDE:
			case PRESET_TOP_WIDE:
			case PRESET_RIGHT_WIDE:
			case PRESET_BOTTOM_WIDE:
			case PRESET_VCENTER_WIDE:
			case PRESET_HCENTER_WIDE:
			case PRESET_WIDE:
				EditorNode::get_undo_redo()->add_do_method(control, "set_margins_preset", p_preset, Control::PRESET_MODE_MINSIZE);
				break;
		}
		EditorNode::get_undo_redo()->add_undo_method(control, "_edit_set_state", control->_edit_get_state());
	}

	EditorNode::get_undo_redo()->commit_action();

	ControlEditorPlugin::get_singleton()->set_anchors_mode(false);
	anchor_mode_button->set_pressed(false);
}

void AnchorAndMarginsEditor::_set_anchors_and_margins_to_keep_ratio() {
	EditorNode::get_undo_redo()->create_action(TTR("Change Anchors and Margins"));

	for (List<Control *>::Element *E = controls.front(); E; E = E->next()) {
		Control *control = E->get();

		Point2 top_left_anchor = _position_to_anchor(control, Point2());
		Point2 bottom_right_anchor = _position_to_anchor(control, control->get_size());

		EditorNode::get_undo_redo()->add_do_method(control, "set_anchor", MARGIN_LEFT, top_left_anchor.x, false, true);
		EditorNode::get_undo_redo()->add_do_method(control, "set_anchor", MARGIN_RIGHT, bottom_right_anchor.x, false, true);
		EditorNode::get_undo_redo()->add_do_method(control, "set_anchor", MARGIN_TOP, top_left_anchor.y, false, true);
		EditorNode::get_undo_redo()->add_do_method(control, "set_anchor", MARGIN_BOTTOM, bottom_right_anchor.y, false, true);
		EditorNode::get_undo_redo()->add_do_method(control, "set_meta", "_edit_use_anchors_", true);

		bool use_anchors = control->has_meta("_edit_use_anchors_") && control->get_meta("_edit_use_anchors_");
		EditorNode::get_undo_redo()->add_undo_method(control, "_edit_set_state", control->_edit_get_state());
		EditorNode::get_undo_redo()->add_undo_method(control, "set_meta", "_edit_use_anchors_", use_anchors);

		ControlEditorPlugin::get_singleton()->set_anchors_mode(true);
		anchor_mode_button->set_pressed(true);
	}

	EditorNode::get_undo_redo()->commit_action();
}

void AnchorAndMarginsEditor::_set_anchors_preset(Control::LayoutPreset p_preset) {
	EditorNode::get_undo_redo()->create_action(TTR("Change Anchors"));

	for (List<Control *>::Element *E = controls.front(); E; E = E->next()) {
		Control *control = E->get();

		EditorNode::get_undo_redo()->add_do_method(control, "set_anchors_preset", p_preset);
		EditorNode::get_undo_redo()->add_undo_method(control, "_edit_set_state", control->_edit_get_state());
	}

	EditorNode::get_undo_redo()->commit_action();
}

void AnchorAndMarginsEditor::_popup_callback(int p_op) {

	switch (p_op) {
		case ANCHORS_AND_MARGINS_PRESET_TOP_LEFT: {
			_set_anchors_and_margins_preset(Control::PRESET_TOP_LEFT);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_TOP_RIGHT: {
			_set_anchors_and_margins_preset(Control::PRESET_TOP_RIGHT);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_BOTTOM_LEFT: {
			_set_anchors_and_margins_preset(Control::PRESET_BOTTOM_LEFT);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_BOTTOM_RIGHT: {
			_set_anchors_and_margins_preset(Control::PRESET_BOTTOM_RIGHT);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_CENTER_LEFT: {
			_set_anchors_and_margins_preset(Control::PRESET_CENTER_LEFT);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_CENTER_RIGHT: {
			_set_anchors_and_margins_preset(Control::PRESET_CENTER_RIGHT);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_CENTER_TOP: {
			_set_anchors_and_margins_preset(Control::PRESET_CENTER_TOP);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_CENTER_BOTTOM: {
			_set_anchors_and_margins_preset(Control::PRESET_CENTER_BOTTOM);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_CENTER: {
			_set_anchors_and_margins_preset(Control::PRESET_CENTER);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_TOP_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_TOP_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_LEFT_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_LEFT_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_RIGHT_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_RIGHT_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_BOTTOM_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_BOTTOM_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_VCENTER_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_VCENTER_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_HCENTER_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_HCENTER_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_WIDE: {
			_set_anchors_and_margins_preset(Control::PRESET_WIDE);
		} break;
		case ANCHORS_AND_MARGINS_PRESET_KEEP_RATIO: {
			_set_anchors_and_margins_to_keep_ratio();
		} break;
		case ANCHORS_PRESET_TOP_LEFT: {
			_set_anchors_preset(Control::PRESET_TOP_LEFT);
		} break;
		case ANCHORS_PRESET_TOP_RIGHT: {
			_set_anchors_preset(Control::PRESET_TOP_RIGHT);
		} break;
		case ANCHORS_PRESET_BOTTOM_LEFT: {
			_set_anchors_preset(Control::PRESET_BOTTOM_LEFT);
		} break;
		case ANCHORS_PRESET_BOTTOM_RIGHT: {
			_set_anchors_preset(Control::PRESET_BOTTOM_RIGHT);
		} break;
		case ANCHORS_PRESET_CENTER_LEFT: {
			_set_anchors_preset(Control::PRESET_CENTER_LEFT);
		} break;
		case ANCHORS_PRESET_CENTER_RIGHT: {
			_set_anchors_preset(Control::PRESET_CENTER_RIGHT);
		} break;
		case ANCHORS_PRESET_CENTER_TOP: {
			_set_anchors_preset(Control::PRESET_CENTER_TOP);
		} break;
		case ANCHORS_PRESET_CENTER_BOTTOM: {
			_set_anchors_preset(Control::PRESET_CENTER_BOTTOM);
		} break;
		case ANCHORS_PRESET_CENTER: {
			_set_anchors_preset(Control::PRESET_CENTER);
		} break;
		case ANCHORS_PRESET_TOP_WIDE: {
			_set_anchors_preset(Control::PRESET_TOP_WIDE);
		} break;
		case ANCHORS_PRESET_LEFT_WIDE: {
			_set_anchors_preset(Control::PRESET_LEFT_WIDE);
		} break;
		case ANCHORS_PRESET_RIGHT_WIDE: {
			_set_anchors_preset(Control::PRESET_RIGHT_WIDE);
		} break;
		case ANCHORS_PRESET_BOTTOM_WIDE: {
			_set_anchors_preset(Control::PRESET_BOTTOM_WIDE);
		} break;
		case ANCHORS_PRESET_VCENTER_WIDE: {
			_set_anchors_preset(Control::PRESET_VCENTER_WIDE);
		} break;
		case ANCHORS_PRESET_HCENTER_WIDE: {
			_set_anchors_preset(Control::PRESET_HCENTER_WIDE);
		} break;
		case ANCHORS_PRESET_WIDE: {
			_set_anchors_preset(Control::PRESET_WIDE);
		} break;
	}
}

/*void CanvasItemEditor::_selection_changed() {
	// Update the anchors_mode
	int nbValidControls = 0;
	int nbAnchorsMode = 0;
	List<Node *> selection = editor_selection->get_selected_node_list();
	for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
		Control *control = Object::cast_to<Control>(E->get());
		if (!control)
			continue;
		if (Object::cast_to<Container>(control->get_parent()))
			continue;

		nbValidControls++;
		if (control->has_meta("_edit_use_anchors_") && control->get_meta("_edit_use_anchors_")) {
			nbAnchorsMode++;
		}
	}
	anchors_mode = (nbValidControls == nbAnchorsMode);
	anchor_mode_button->set_pressed(anchors_mode);

	if (!selected_from_canvas) {
		drag_type = DRAG_NONE;
	}
	selected_from_canvas = false;
}*/

void AnchorAndMarginsEditor::_button_toggle_anchor_mode(bool p_status) {

	// Set the control metadata for the anchors_mode
	for (List<Control *>::Element *E = controls.front(); E; E = E->next()) {
		Control *control = E->get();

		if (!control || Object::cast_to<Container>(control->get_parent()))
			continue;

		control->set_meta("_edit_use_anchors_", p_status);
	}

	ControlEditorPlugin::get_singleton()->set_anchors_mode(p_status);
}

bool AnchorAndMarginsEditor::_get_control_anchor_preset(const Control *p_control, Control::LayoutPreset &r_layout_preset) {
	bool anchor_left_zero = p_control->get_anchor(MARGIN_LEFT) == 0.0;
	bool anchor_left_center = p_control->get_anchor(MARGIN_LEFT) == 0.5;
	bool anchor_left_one = p_control->get_anchor(MARGIN_LEFT) == 1.0;

	bool anchor_right_zero = p_control->get_anchor(MARGIN_RIGHT) == 0.0;
	bool anchor_right_center = p_control->get_anchor(MARGIN_RIGHT) == 0.5;
	bool anchor_right_one = p_control->get_anchor(MARGIN_RIGHT) == 1.0;

	bool anchor_top_zero = p_control->get_anchor(MARGIN_TOP) == 0.0;
	bool anchor_top_center = p_control->get_anchor(MARGIN_TOP) == 0.5;
	bool anchor_top_one = p_control->get_anchor(MARGIN_TOP) == 1.0;

	bool anchor_bottom_zero = p_control->get_anchor(MARGIN_BOTTOM) == 0.0;
	bool anchor_bottom_center = p_control->get_anchor(MARGIN_BOTTOM) == 0.5;
	bool anchor_bottom_one = p_control->get_anchor(MARGIN_BOTTOM) == 1.0;

	if (anchor_top_zero && anchor_bottom_zero) {
		if (anchor_left_zero && anchor_right_zero) {
			r_layout_preset = PRESET_TOP_LEFT;
			return true;
		} else if (anchor_left_center && anchor_right_center) {
			r_layout_preset = PRESET_CENTER_TOP;
			return true;
		} else if (anchor_left_one && anchor_right_one) {
			r_layout_preset = PRESET_TOP_RIGHT;
			return true;
		} else if (anchor_left_zero && anchor_right_one) {
			r_layout_preset = PRESET_TOP_WIDE;
			return true;
		}
	} else if (anchor_top_center && anchor_bottom_center) {
		if (anchor_left_zero && anchor_right_zero) {
			r_layout_preset = PRESET_CENTER_LEFT;
			return true;
		} else if (anchor_left_center && anchor_right_center) {
			r_layout_preset = PRESET_CENTER;
			return true;
		} else if (anchor_left_one && anchor_right_one) {
			r_layout_preset = PRESET_CENTER_RIGHT;
			return true;
		} else if (anchor_left_zero && anchor_right_one) {
			r_layout_preset = PRESET_HCENTER_WIDE;
			return true;
		}
	} else if (anchor_top_one && anchor_bottom_one) {
		if (anchor_left_zero && anchor_right_zero) {
			r_layout_preset = PRESET_BOTTOM_LEFT;
			return true;
		} else if (anchor_left_center && anchor_right_center) {
			r_layout_preset = PRESET_CENTER_BOTTOM;
			return true;
		} else if (anchor_left_one && anchor_right_one) {
			r_layout_preset = PRESET_BOTTOM_RIGHT;
			return true;
		} else if (anchor_left_zero && anchor_right_one) {
			r_layout_preset = PRESET_BOTTOM_WIDE;
			return true;
		}
	} else if (anchor_top_zero && anchor_bottom_one) {
		if (anchor_left_zero && anchor_right_zero) {
			r_layout_preset = PRESET_LEFT_WIDE;
			return true;
		} else if (anchor_left_center && anchor_right_center) {
			r_layout_preset = PRESET_VCENTER_WIDE;
			return true;
		} else if (anchor_left_one && anchor_right_one) {
			r_layout_preset = PRESET_RIGHT_WIDE;
			return true;
		} else if (anchor_left_zero && anchor_right_one) {
			r_layout_preset = PRESET_WIDE;
			return true;
		}
	}
	return false;
}

bool AnchorAndMarginsEditor::_get_control_anchor_and_margin_preset(const Control *p_control, Control::LayoutPreset &r_layout_preset) {
	// Get the anchor preset first, return if none
	Control::LayoutPreset anchors_layout_preset;
	bool result = _get_control_anchor_preset(p_control, anchors_layout_preset);
	if (!result)
		return false;

	bool margin_left_zero = p_control->get_margin(MARGIN_LEFT) == 0.0;
	bool margin_left_center = p_control->get_margin(MARGIN_LEFT) == 0.5;
	bool margin_left_one = p_control->get_margin(MARGIN_LEFT) == 1.0;

	bool margin_right_zero = p_control->get_margin(MARGIN_RIGHT) == 0.0;
	bool margin_right_center = p_control->get_margin(MARGIN_RIGHT) == 0.5;
	bool margin_right_one = p_control->get_margin(MARGIN_RIGHT) == 1.0;

	bool margin_top_zero = p_control->get_margin(MARGIN_TOP) == 0.0;
	bool margin_top_center = p_control->get_margin(MARGIN_TOP) == 0.5;
	bool margin_top_one = p_control->get_margin(MARGIN_TOP) == 1.0;

	bool margin_bottom_zero = p_control->get_margin(MARGIN_BOTTOM) == 0.0;
	bool margin_bottom_center = p_control->get_margin(MARGIN_BOTTOM) == 0.5;
	bool margin_bottom_one = p_control->get_margin(MARGIN_BOTTOM) == 1.0;

	if (margin_top_zero && margin_bottom_zero) {
		if (margin_left_zero && margin_right_zero && anchors_layout_preset == ANCHORS_PRESET_TOP_LEFT) {
			r_layout_preset = PRESET_TOP_LEFT;
			return true;
		} else if (margin_left_center && margin_right_center && anchors_layout_preset == ANCHORS_PRESET_CENTER_TOP) {
			r_layout_preset = PRESET_CENTER_TOP;
			return true;
		} else if (margin_left_one && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_TOP_RIGHT) {
			r_layout_preset = PRESET_TOP_RIGHT;
			return true;
		} else if (margin_left_zero && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_TOP_WIDE) {
			r_layout_preset = PRESET_TOP_WIDE;
			return true;
		}
	} else if (margin_top_center && margin_bottom_center) {
		if (margin_left_zero && margin_right_zero && anchors_layout_preset == ANCHORS_PRESET_CENTER_LEFT) {
			r_layout_preset = PRESET_CENTER_LEFT;
			return true;
		} else if (margin_left_center && margin_right_center && anchors_layout_preset == ANCHORS_PRESET_CENTER) {
			r_layout_preset = PRESET_CENTER;
			return true;
		} else if (margin_left_one && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_CENTER_RIGHT) {
			r_layout_preset = PRESET_CENTER_RIGHT;
			return true;
		} else if (margin_left_zero && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_HCENTER_WIDE) {
			r_layout_preset = PRESET_HCENTER_WIDE;
			return true;
		}
	} else if (margin_top_one && margin_bottom_one) {
		if (margin_left_zero && margin_right_zero && anchors_layout_preset == ANCHORS_PRESET_BOTTOM_RIGHT) {
			r_layout_preset = PRESET_BOTTOM_LEFT;
			return true;
		} else if (margin_left_center && margin_right_center && anchors_layout_preset == ANCHORS_PRESET_CENTER_BOTTOM) {
			r_layout_preset = PRESET_CENTER_BOTTOM;
			return true;
		} else if (margin_left_one && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_BOTTOM_LEFT) {
			r_layout_preset = PRESET_BOTTOM_RIGHT;
			return true;
		} else if (margin_left_zero && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_BOTTOM_WIDE) {
			r_layout_preset = PRESET_BOTTOM_WIDE;
			return true;
		}
	} else if (margin_top_zero && margin_bottom_one) {
		if (margin_left_zero && margin_right_zero && anchors_layout_preset == ANCHORS_PRESET_LEFT_WIDE) {
			r_layout_preset = PRESET_LEFT_WIDE;
			return true;
		} else if (margin_left_center && margin_right_center && anchors_layout_preset == ANCHORS_PRESET_VCENTER_WIDE) {
			r_layout_preset = PRESET_VCENTER_WIDE;
			return true;
		} else if (margin_left_one && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_RIGHT_WIDE) {
			r_layout_preset = PRESET_RIGHT_WIDE;
			return true;
		} else if (margin_left_zero && margin_right_one && anchors_layout_preset == ANCHORS_PRESET_WIDE) {
			r_layout_preset = PRESET_WIDE;
			return true;
		}
	}
	return false;
}

void AnchorAndMarginsEditor::_update_icon_and_text() {
	Control::LayoutPreset anchors_layout_preset;
	bool result = _get_control_anchor_preset(controls.front()->get() /* FIXME: compute if common to all controls */, anchors_layout_preset);
	if (!result) {
		presets->set_icon(get_icon("ControlLayout", "EditorIcons"));
		presets->set_text(TTR("Custom layout"));
		return;
	}

	switch (anchors_layout_preset) {
		case PRESET_TOP_LEFT:
			presets->set_icon(get_icon("ControlAlignTopLeft", "EditorIcons"));
			presets->set_text(TTR("Top Left"));
			break;
		case PRESET_TOP_RIGHT:
			presets->set_icon(get_icon("ControlAlignTopRight", "EditorIcons"));
			presets->set_text(TTR("Top Right"));
			break;
		case PRESET_BOTTOM_LEFT:
			presets->set_icon(get_icon("ControlAlignBottomLeft", "EditorIcons"));
			presets->set_text(TTR("Bottom Right"));
			break;
		case PRESET_BOTTOM_RIGHT:
			presets->set_icon(get_icon("ControlAlignBottomRight", "EditorIcons"));
			presets->set_text(TTR("Bottom Left"));
			break;
		case PRESET_CENTER_LEFT:
			presets->set_icon(get_icon("ControlAlignLeftCenter", "EditorIcons"));
			presets->set_text(TTR("Center Left"));
			break;
		case PRESET_CENTER_TOP:
			presets->set_icon(get_icon("ControlAlignTopCenter", "EditorIcons"));
			presets->set_text(TTR("Center Top"));
			break;
		case PRESET_CENTER_RIGHT:
			presets->set_icon(get_icon("ControlAlignRightCenter", "EditorIcons"));
			presets->set_text(TTR("Center Right"));
			break;
		case PRESET_CENTER_BOTTOM:
			presets->set_icon(get_icon("ControlAlignBottomCenter", "EditorIcons"));
			presets->set_text(TTR("Center Bottom"));
			break;
		case PRESET_CENTER:
			presets->set_icon(get_icon("ControlAlignCenter", "EditorIcons"));
			presets->set_text(TTR("Center"));
			break;
		case PRESET_LEFT_WIDE:
			presets->set_icon(get_icon("ControlAlignLeftWide", "EditorIcons"));
			presets->set_text(TTR("Left Wide"));
			break;
		case PRESET_TOP_WIDE:
			presets->set_icon(get_icon("ControlAlignTopWide", "EditorIcons"));
			presets->set_text(TTR("Top Wide"));
			break;
		case PRESET_RIGHT_WIDE:
			presets->set_icon(get_icon("ControlAlignRightWide", "EditorIcons"));
			presets->set_text(TTR("Right Wide"));
			break;
		case PRESET_BOTTOM_WIDE:
			presets->set_icon(get_icon("ControlAlignBottomWide", "EditorIcons"));
			presets->set_text(TTR("Bottom Wide"));
			break;
		case PRESET_VCENTER_WIDE:
			presets->set_icon(get_icon("ControlVcenterWide", "EditorIcons"));
			presets->set_text(TTR("VCenter Wide"));
			break;
		case PRESET_HCENTER_WIDE:
			presets->set_icon(get_icon("ControlHcenterWide", "EditorIcons"));
			presets->set_text(TTR("HCenter Wide"));
			break;
		case PRESET_WIDE:
			presets->set_icon(get_icon("ControlAlignWide", "EditorIcons"));
			presets->set_text(TTR("Full Rect"));
			break;
	}
}

void AnchorAndMarginsEditor::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE || p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {
		_update_icon_and_text();

		PopupMenu *p = presets->get_popup();
		p->clear();
		p->add_icon_item(get_icon("ControlAlignTopLeft", "EditorIcons"), TTR("Top Left"), ANCHORS_AND_MARGINS_PRESET_TOP_LEFT);
		p->add_icon_item(get_icon("ControlAlignTopRight", "EditorIcons"), TTR("Top Right"), ANCHORS_AND_MARGINS_PRESET_TOP_RIGHT);
		p->add_icon_item(get_icon("ControlAlignBottomRight", "EditorIcons"), TTR("Bottom Right"), ANCHORS_AND_MARGINS_PRESET_BOTTOM_RIGHT);
		p->add_icon_item(get_icon("ControlAlignBottomLeft", "EditorIcons"), TTR("Bottom Left"), ANCHORS_AND_MARGINS_PRESET_BOTTOM_LEFT);
		p->add_separator();
		p->add_icon_item(get_icon("ControlAlignLeftCenter", "EditorIcons"), TTR("Center Left"), ANCHORS_AND_MARGINS_PRESET_CENTER_LEFT);
		p->add_icon_item(get_icon("ControlAlignTopCenter", "EditorIcons"), TTR("Center Top"), ANCHORS_AND_MARGINS_PRESET_CENTER_TOP);
		p->add_icon_item(get_icon("ControlAlignRightCenter", "EditorIcons"), TTR("Center Right"), ANCHORS_AND_MARGINS_PRESET_CENTER_RIGHT);
		p->add_icon_item(get_icon("ControlAlignBottomCenter", "EditorIcons"), TTR("Center Bottom"), ANCHORS_AND_MARGINS_PRESET_CENTER_BOTTOM);
		p->add_icon_item(get_icon("ControlAlignCenter", "EditorIcons"), TTR("Center"), ANCHORS_AND_MARGINS_PRESET_CENTER);
		p->add_separator();
		p->add_icon_item(get_icon("ControlAlignLeftWide", "EditorIcons"), TTR("Left Wide"), ANCHORS_AND_MARGINS_PRESET_LEFT_WIDE);
		p->add_icon_item(get_icon("ControlAlignTopWide", "EditorIcons"), TTR("Top Wide"), ANCHORS_AND_MARGINS_PRESET_TOP_WIDE);
		p->add_icon_item(get_icon("ControlAlignRightWide", "EditorIcons"), TTR("Right Wide"), ANCHORS_AND_MARGINS_PRESET_RIGHT_WIDE);
		p->add_icon_item(get_icon("ControlAlignBottomWide", "EditorIcons"), TTR("Bottom Wide"), ANCHORS_AND_MARGINS_PRESET_BOTTOM_WIDE);
		p->add_icon_item(get_icon("ControlVcenterWide", "EditorIcons"), TTR("VCenter Wide"), ANCHORS_AND_MARGINS_PRESET_VCENTER_WIDE);
		p->add_icon_item(get_icon("ControlHcenterWide", "EditorIcons"), TTR("HCenter Wide"), ANCHORS_AND_MARGINS_PRESET_HCENTER_WIDE);
		p->add_separator();
		p->add_icon_item(get_icon("ControlAlignWide", "EditorIcons"), TTR("Full Rect"), ANCHORS_AND_MARGINS_PRESET_WIDE);
		p->add_icon_item(get_icon("Anchor", "EditorIcons"), TTR("Keep Ratio"), ANCHORS_AND_MARGINS_PRESET_KEEP_RATIO);
		p->add_separator();
		p->add_submenu_item(TTR("Anchors only"), "Anchors");
		p->set_item_icon(21, get_icon("Anchor", "EditorIcons"));

		anchors_popup->clear();
		anchors_popup->add_icon_item(get_icon("ControlAlignTopLeft", "EditorIcons"), TTR("Top Left"), ANCHORS_PRESET_TOP_LEFT);
		anchors_popup->add_icon_item(get_icon("ControlAlignTopRight", "EditorIcons"), TTR("Top Right"), ANCHORS_PRESET_TOP_RIGHT);
		anchors_popup->add_icon_item(get_icon("ControlAlignBottomRight", "EditorIcons"), TTR("Bottom Right"), ANCHORS_PRESET_BOTTOM_RIGHT);
		anchors_popup->add_icon_item(get_icon("ControlAlignBottomLeft", "EditorIcons"), TTR("Bottom Left"), ANCHORS_PRESET_BOTTOM_LEFT);
		anchors_popup->add_separator();
		anchors_popup->add_icon_item(get_icon("ControlAlignLeftCenter", "EditorIcons"), TTR("Center Left"), ANCHORS_PRESET_CENTER_LEFT);
		anchors_popup->add_icon_item(get_icon("ControlAlignTopCenter", "EditorIcons"), TTR("Center Top"), ANCHORS_PRESET_CENTER_TOP);
		anchors_popup->add_icon_item(get_icon("ControlAlignRightCenter", "EditorIcons"), TTR("Center Right"), ANCHORS_PRESET_CENTER_RIGHT);
		anchors_popup->add_icon_item(get_icon("ControlAlignBottomCenter", "EditorIcons"), TTR("Center Bottom"), ANCHORS_PRESET_CENTER_BOTTOM);
		anchors_popup->add_icon_item(get_icon("ControlAlignCenter", "EditorIcons"), TTR("Center"), ANCHORS_PRESET_CENTER);
		anchors_popup->add_separator();
		anchors_popup->add_icon_item(get_icon("ControlAlignLeftWide", "EditorIcons"), TTR("Left Wide"), ANCHORS_PRESET_LEFT_WIDE);
		anchors_popup->add_icon_item(get_icon("ControlAlignTopWide", "EditorIcons"), TTR("Top Wide"), ANCHORS_PRESET_TOP_WIDE);
		anchors_popup->add_icon_item(get_icon("ControlAlignRightWide", "EditorIcons"), TTR("Right Wide"), ANCHORS_PRESET_RIGHT_WIDE);
		anchors_popup->add_icon_item(get_icon("ControlAlignBottomWide", "EditorIcons"), TTR("Bottom Wide"), ANCHORS_PRESET_BOTTOM_WIDE);
		anchors_popup->add_icon_item(get_icon("ControlVcenterWide", "EditorIcons"), TTR("VCenter Wide"), ANCHORS_PRESET_VCENTER_WIDE);
		anchors_popup->add_icon_item(get_icon("ControlHcenterWide", "EditorIcons"), TTR("HCenter Wide"), ANCHORS_PRESET_HCENTER_WIDE);
		anchors_popup->add_separator();
		anchors_popup->add_icon_item(get_icon("ControlAlignWide", "EditorIcons"), TTR("Full Rect"), ANCHORS_PRESET_WIDE);

		anchor_mode_button->set_icon(get_icon("Anchor", "EditorIcons"));

	} else if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
		// Disable if the selected node is child of a container
		bool has_container_parents = false;
		/*if (Object::cast_to<Container>(control->get_parent())) {
			has_container_parents = true;
		}*/
		if (has_container_parents) {
			anchor_mode_button->set_disabled(true);
			anchor_mode_button->set_tooltip(TTR("Children of containers have their anchors and margins values overridden by their parent."));
		} else {
			anchor_mode_button->set_disabled(false);
			anchor_mode_button->set_tooltip(TTR("When active, moving Control nodes changes their anchors instead of their margins."));
		}
	}
}

void AnchorAndMarginsEditor::_changed_callback(Object *p_changed, const char *p_prop) {
	_update_icon_and_text();
}

void AnchorAndMarginsEditor::_bind_methods() {
	ClassDB::bind_method("_button_toggle_anchor_mode", &AnchorAndMarginsEditor::_button_toggle_anchor_mode);
	ClassDB::bind_method("_popup_callback", &AnchorAndMarginsEditor::_popup_callback);
}

void AnchorAndMarginsEditor::set_object(Object *p_object) {
	controls.clear();
	Control *control = Object::cast_to<Control>(p_object);
	MultiNodeEdit *multi_node_edit = Object::cast_to<MultiNodeEdit>(p_object);
	if (control) {
		controls.push_back(control);
	} else if (multi_node_edit) {
		// TODO
	}

	for (List<Control *>::Element *E = controls.front(); E; E = E->next()) {
		E->get()->add_change_receptor(this);
	}
}

AnchorAndMarginsEditor::AnchorAndMarginsEditor() {
	HBoxContainer *toolbar = memnew(HBoxContainer);
	add_child(toolbar);

	presets = memnew(MenuButton);
	presets->set_switch_on_hover(true);
	toolbar->add_child(presets);
	PopupMenu *p = presets->get_popup();
	p->connect("id_pressed", this, "_popup_callback");

	anchor_mode_button = memnew(ToolButton);
	toolbar->add_child(anchor_mode_button);
	anchor_mode_button->set_toggle_mode(true);
	anchor_mode_button->connect("toggled", this, "_button_toggle_anchor_mode");

	anchors_popup = memnew(PopupMenu);
	p->add_child(anchors_popup);
	anchors_popup->set_name("Anchors");
	anchors_popup->connect("id_pressed", this, "_popup_callback");
}

// --------------------
bool EditorInspectorPluginControl::can_handle(Object *p_object) {

	return Object::cast_to<Control>(p_object) != NULL;
}

void EditorInspectorPluginControl::parse_begin(Object *p_object) {
	// Do nothing
}

bool EditorInspectorPluginControl::parse_property(Object *p_object, Variant::Type p_type, const String &p_path, PropertyHint p_hint, const String &p_hint_text, int p_usage) {
	if (p_path == replaced_properties.get(0)) {
		anchor_and_margins_editor = memnew(AnchorAndMarginsEditor);
		anchor_and_margins_editor->set_object(p_object);
		add_property_editor_for_multiple_properties("Anchors and margins", replaced_properties, anchor_and_margins_editor);
	}

	bool in_replaced_properties = (replaced_properties.find(p_path) >= 0);
	if (in_replaced_properties) {
		EditorProperty *editor = EditorInspectorDefaultPlugin::get_default_editor_for_property(p_type, p_path, p_hint, p_hint_text, p_usage);
		add_property_editor(p_path, editor);
	}

	return in_replaced_properties;
}

void EditorInspectorPluginControl::parse_end() {
	// Do nothing
}

EditorInspectorPluginControl::EditorInspectorPluginControl() {

	replaced_properties.push_back("anchor_left");
	replaced_properties.push_back("anchor_right");
	replaced_properties.push_back("anchor_top");
	replaced_properties.push_back("anchor_bottom");
	replaced_properties.push_back("margin_left");
	replaced_properties.push_back("margin_right");
	replaced_properties.push_back("margin_top");
	replaced_properties.push_back("margin_bottom");
}

ControlEditorPlugin *ControlEditorPlugin::singleton = NULL;

ControlEditorPlugin::ControlEditorPlugin(EditorNode *p_node) {
	anchors_mode = false;

	// Add the inspector plugin
	Ref<EditorInspectorPluginControl> plugin;
	plugin.instance();
	add_inspector_plugin(plugin);

	// Set the singleton
	singleton = this;
}
