/*************************************************************************/
/*   control_editor_plugin.h                                             */
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

#ifndef CONTROL_EDITOR_PLUGIN_H
#define CONTROL_EDITOR_PLUGIN_H

#include "core/vector.h"
#include "editor/editor_plugin.h"
#include "scene/gui/control.h"

class AnchorAndMarginsEditor : public VBoxContainer {
	GDCLASS(AnchorAndMarginsEditor, VBoxContainer);

private:
	enum MenuOption {
		ANCHORS_AND_MARGINS_PRESET_TOP_LEFT,
		ANCHORS_AND_MARGINS_PRESET_TOP_RIGHT,
		ANCHORS_AND_MARGINS_PRESET_BOTTOM_LEFT,
		ANCHORS_AND_MARGINS_PRESET_BOTTOM_RIGHT,
		ANCHORS_AND_MARGINS_PRESET_CENTER_LEFT,
		ANCHORS_AND_MARGINS_PRESET_CENTER_RIGHT,
		ANCHORS_AND_MARGINS_PRESET_CENTER_TOP,
		ANCHORS_AND_MARGINS_PRESET_CENTER_BOTTOM,
		ANCHORS_AND_MARGINS_PRESET_CENTER,
		ANCHORS_AND_MARGINS_PRESET_TOP_WIDE,
		ANCHORS_AND_MARGINS_PRESET_LEFT_WIDE,
		ANCHORS_AND_MARGINS_PRESET_RIGHT_WIDE,
		ANCHORS_AND_MARGINS_PRESET_BOTTOM_WIDE,
		ANCHORS_AND_MARGINS_PRESET_VCENTER_WIDE,
		ANCHORS_AND_MARGINS_PRESET_HCENTER_WIDE,
		ANCHORS_AND_MARGINS_PRESET_WIDE,
		ANCHORS_AND_MARGINS_PRESET_KEEP_RATIO,
		ANCHORS_PRESET_TOP_LEFT,
		ANCHORS_PRESET_TOP_RIGHT,
		ANCHORS_PRESET_BOTTOM_LEFT,
		ANCHORS_PRESET_BOTTOM_RIGHT,
		ANCHORS_PRESET_CENTER_LEFT,
		ANCHORS_PRESET_CENTER_RIGHT,
		ANCHORS_PRESET_CENTER_TOP,
		ANCHORS_PRESET_CENTER_BOTTOM,
		ANCHORS_PRESET_CENTER,
		ANCHORS_PRESET_TOP_WIDE,
		ANCHORS_PRESET_LEFT_WIDE,
		ANCHORS_PRESET_RIGHT_WIDE,
		ANCHORS_PRESET_BOTTOM_WIDE,
		ANCHORS_PRESET_VCENTER_WIDE,
		ANCHORS_PRESET_HCENTER_WIDE,
		ANCHORS_PRESET_WIDE
	};

	List<Control *> controls;

	MenuButton *presets;
	PopupMenu *anchors_popup;
	void _popup_callback(int p_op);

	void _button_toggle_anchor_mode(bool p_status);
	ToolButton *anchor_mode_button;

	void _set_anchors_preset(Control::LayoutPreset p_preset);
	void _set_anchors_and_margins_preset(Control::LayoutPreset p_preset);
	void _set_anchors_and_margins_to_keep_ratio();

	Vector2 _position_to_anchor(const Control *p_control, Vector2 position);

	bool _get_control_anchor_preset(const Control *p_control, Control::LayoutPreset &r_layout_preset);
	bool _get_control_anchor_and_margin_preset(const Control *p_control, Control::LayoutPreset &r_layout_preset);
	void _update_icon_and_text();

	HashMap<StringName, Control *> property_editors;

protected:
	virtual void _changed_callback(Object *p_changed, const char *p_prop);

	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_object(Object *p_object);

	AnchorAndMarginsEditor();
};

class EditorInspectorPluginControl : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginControl, EditorInspectorPlugin);

private:
	Vector<String> replaced_properties;
	AnchorAndMarginsEditor *anchor_and_margins_editor;

public:
	virtual bool can_handle(Object *p_object);
	virtual void parse_begin(Object *p_object);
	virtual void parse_end();
	virtual bool parse_property(Object *p_object, Variant::Type p_type, const String &p_path, PropertyHint p_hint, const String &p_hint_text, int p_usage);

	EditorInspectorPluginControl();
};

class ControlEditorPlugin : public EditorPlugin {
	GDCLASS(ControlEditorPlugin, EditorPlugin);

private:
	bool anchors_mode;

protected:
	static ControlEditorPlugin *singleton;

public:
	static ControlEditorPlugin *get_singleton() { return singleton; }

	void set_anchors_mode(bool p_anchors_mode) { anchors_mode = p_anchors_mode; }
	bool get_anchors_mode() { return anchors_mode; }

	virtual String get_name() const { return "Control"; }

	ControlEditorPlugin(EditorNode *p_node);
};

#endif // CONTROL_EDITOR_PLUGIN_H
