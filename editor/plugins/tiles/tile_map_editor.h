/*************************************************************************/
/*  tile_map_editor.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef TILE_MAP_EDITOR_H
#define TILE_MAP_EDITOR_H

#include "tile_atlas_view.h"

#include "scene/2d/tile_map.h"
#include "scene/gui/box_container.h"

#include "editor/editor_node.h"

class TileMapEditor : public VBoxContainer {
	GDCLASS(TileMapEditor, VBoxContainer);

private:
	struct TilePattern {
		Vector2i size;
		Map<Vector2i, TileMapCell> pattern;
	};

	UndoRedo *undo_redo = EditorNode::get_undo_redo();

	bool tileset_changed_needs_update = false;
	bool has_mouse = false;

	// --- TileSet ---
	Label *missing_tileset_label;
	TabContainer *tileset_tabs_container;
	HSplitContainer *atlas_sources_split_container;
	Label *missing_atlas_source_label;
	void _update_bottom_panel();

	ItemList *sources_list;
	TileAtlasView *tile_atlas_view;
	Ref<Texture2D> missing_texture_texture;
	void _update_tile_set_atlas_sources_list();
	void _update_atlas_view();

	void _update_fix_selected_and_hovered();

	TilePattern _get_pattern_from_set(const Set<TileMapCell> &p_set);
	bool tile_set_dragging_selection = false;
	Vector2i tile_set_drag_start_mouse_pos;
	Set<TileMapCell> tile_set_selection;
	TileMapCell hovered_tile;

	Control *tile_atlas_control;
	void _tile_atlas_control_mouse_exited();
	void _tile_atlas_control_gui_input(const Ref<InputEvent> &p_event);
	void _tile_atlas_control_draw();

	Control *alternative_tiles_control;
	void _tile_alternatives_control_draw();
	void _tile_alternatives_control_mouse_exited();
	void _tile_alternatives_control_gui_input(const Ref<InputEvent> &p_event);

	// --- TileMap ---
	TileMap *tile_map = nullptr;

	HBoxContainer *tilemap_toolbar;

	Ref<ButtonGroup> tilemap_tool_buttons_group;
	Button *tilemap_select_tool_button;
	Button *tilemap_paint_tool_button;
	Button *tilemap_line_tool_button;
	Button *tilemap_rect_tool_button;
	Button *tilemap_bucket_tool_button;
	Button *tilemap_picker_tool_button;

	HBoxContainer *tilemap_tools_settings;
	VSeparator *tilemap_tools_settings_vsep;
	Button *erase_button;

	void _update_toolbar();

	Ref<Texture2D> missing_tile_texture;
	Ref<Texture2D> warning_pattern_texture;

	bool tile_map_dragging_selection = false;
	enum DragType {
		DRAG_TYPE_NONE = 0,
		DRAG_TYPE_PAINT,
		DRAG_TYPE_LINE
	};
	DragType drag_type = DRAG_TYPE_NONE;
	Vector2 drag_start_mouse_pos;
	Vector2 drag_last_mouse_pos;
	Map<Vector2i, TileMapCell> drag_modified;

	void _mouse_exited_viewport();

	void _draw_line(Vector2 p_last_mouse_pos, Vector2i p_new_mouse_pos);

	void _tile_map_changed();

protected:
	void _notification(int p_what);
	static void _bind_methods();
	void _draw_shape(Control *p_control, Rect2 p_region, TileSet::TileShape p_shape, TileSet::TileOffsetAxis p_offset_axis, Color p_color);

public:
	bool forward_canvas_gui_input(const Ref<InputEvent> &p_event);
	void forward_canvas_draw_over_viewport(Control *p_overlay);

	void edit(TileMap *p_tile_map);
	Control *get_toolbar() { return tilemap_toolbar; };

	TileMapEditor();
};

#endif // TILE_MAP_EDITOR_PLUGIN_H
