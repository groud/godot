/*************************************************************************/
/*  tiles_editor_plugin.cpp                                              */
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

#include "tiles_editor_plugin.h"

#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "editor/plugins/canvas_item_editor_plugin.h"

#include "scene/2d/tile_map.h"
#include "scene/resources/tile_set/tile_set.h"

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/control.h"
#include "scene/gui/separator.h"

#include "tile_set_editor.h"

TilesEditor *TilesEditor::singleton = nullptr;

void TilesEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			tileset_tilemap_switch_button->set_icon(get_theme_icon("TileSet", "EditorIcons"));
		} break;
	}
}

void TilesEditor::_tile_map_changed() {
	tile_set = tile_map->get_tileset();
	_update_switch_button();
	_update_editors();
}

void TilesEditor::_update_switch_button() {
	// Force the buttons status if needed.
	if (tile_map && !tile_set.is_valid()) {
		tileset_tilemap_switch_button->set_pressed(false);
	} else if (!tile_map && tile_set.is_valid()) {
		tileset_tilemap_switch_button->set_pressed(true);
	}
}

void TilesEditor::_update_editors() {
	// Set editors visibility.
	tilemap_toolbar->set_visible(!tileset_tilemap_switch_button->is_pressed());
	tilemap_editor->set_visible(!tileset_tilemap_switch_button->is_pressed());
	tileset_editor->set_visible(tileset_tilemap_switch_button->is_pressed());

	// Enable/disable the switch button.
	if (!tileset_tilemap_switch_button->is_pressed()) {
		if (!tile_set.is_valid()) {
			tileset_tilemap_switch_button->set_disabled(true);
			tileset_tilemap_switch_button->set_tooltip(TTR("This TileMap has no assigned TileSet, assign a TileSet to this TileMap to edit it."));
		} else {
			tileset_tilemap_switch_button->set_disabled(false);
			tileset_tilemap_switch_button->set_tooltip(TTR("Switch between TileSet/TileMap editor."));
		}
	} else {
		if (!tile_map) {
			tileset_tilemap_switch_button->set_disabled(true);
			tileset_tilemap_switch_button->set_tooltip(TTR("You are editing a TileSet resource. Select a TileMap node to paint."));
		} else {
			tileset_tilemap_switch_button->set_disabled(false);
			tileset_tilemap_switch_button->set_tooltip(TTR("Switch between TileSet/TileMap editor."));
		}
	}

	// If tile_map is not edited, we change the edited only if we are not editing a tile_set.
	tilemap_editor->edit(tile_map);
	tileset_editor->edit(*tile_set);

	// Update the viewport
	CanvasItemEditor::get_singleton()->update_viewport();
}

void TilesEditor::set_atlas_sources_lists_current(int p_current) {
	atlas_sources_lists_current = p_current;
}

void TilesEditor::synchronize_atlas_sources_lists(Object *p_current) {
	ItemList *item_list = Object::cast_to<ItemList>(p_current);
	ERR_FAIL_COND(!item_list);

	if (item_list->is_visible_in_tree()) {
		if (atlas_sources_lists_current < 0 || atlas_sources_lists_current >= item_list->get_item_count()) {
			item_list->deselect_all();
		} else {
			item_list->set_current(atlas_sources_lists_current);
			item_list->emit_signal("item_selected", atlas_sources_lists_current);
		}
	}
}

void TilesEditor::_synchronize_atlas_views(Variant p_unused, Object *p_emitter) {
	TileAtlasView *atlas_view = Object::cast_to<TileAtlasView>(p_emitter);
	ERR_FAIL_COND(!atlas_view);

	float zoom = atlas_view->get_zoom();
	Vector2i scroll = Vector2i(atlas_view->get_scroll_container()->get_h_scroll(), atlas_view->get_scroll_container()->get_v_scroll());

	for (Set<TileAtlasView *>::Element *E = atlases_views.front(); E; E = E->next()) {
		// We synchronize only if the scrollable areas are the same.
		E->get()->set_zoom(zoom);
		if (E->get()->get_scroll_container()->get_h_scroll() != scroll.x) {
			E->get()->get_scroll_container()->set_h_scroll(scroll.x);
		}
		if (E->get()->get_scroll_container()->get_v_scroll() != scroll.y) {
			E->get()->get_scroll_container()->set_v_scroll(scroll.y);
		}
	}
}

void TilesEditor::register_atlas_view_for_synchronization(TileAtlasView *p_atlas_view) {
	atlases_views.insert(p_atlas_view);
	p_atlas_view->connect("zoom_changed", callable_mp(this, &TilesEditor::_synchronize_atlas_views), varray(0, Variant(p_atlas_view)));
	p_atlas_view->get_scroll_container()->get_h_scrollbar()->connect("value_changed", callable_mp(this, &TilesEditor::_synchronize_atlas_views), varray(p_atlas_view));
	p_atlas_view->get_scroll_container()->get_v_scrollbar()->connect("value_changed", callable_mp(this, &TilesEditor::_synchronize_atlas_views), varray(p_atlas_view));
}

void TilesEditor::clear() {
	edit(nullptr);
}

void TilesEditor::edit(Object *p_object) {
	// Disconnect.
	if (tile_map) {
		tile_map->disconnect("tree_exiting", callable_mp(this, &TilesEditor::clear));
	}

	// Update edited objects.
	tile_map = nullptr;
	tile_set = Ref<TileSet>();
	if (p_object) {
		if (p_object->is_class("TileMap")) {
			tile_map = (TileMap *)p_object;
			tile_set = tile_map->get_tileset();
		} else if (p_object->is_class("TileSet")) {
			tile_set = Ref<TileSet>(p_object);
		}

		// Update pressed status button.
		if (p_object->is_class("TileMap")) {
			tileset_tilemap_switch_button->set_pressed(false);
		} else if (p_object->is_class("TileSet")) {
			tileset_tilemap_switch_button->set_pressed(true);
		}
	}

	// Update the editors.
	_update_switch_button();
	_update_editors();

	// Add change listener.
	if (tile_map) {
		tile_map->connect("changed", callable_mp(this, &TilesEditor::_tile_map_changed));
		tile_map->connect("tree_exiting", callable_mp(this, &TilesEditor::clear));
	}
}

void TilesEditor::_bind_methods() {
}

TilesEditor::TilesEditor(EditorNode *p_editor) {
	// Update the singleton.
	singleton = this;

	// Toolbar.
	HBoxContainer *toolbar = memnew(HBoxContainer);
	add_child(toolbar);

	// Switch button.
	tileset_tilemap_switch_button = memnew(Button);
	tileset_tilemap_switch_button->set_flat(true);
	tileset_tilemap_switch_button->set_toggle_mode(true);
	tileset_tilemap_switch_button->connect("toggled", callable_mp(this, &TilesEditor::_update_editors).unbind(1));
	toolbar->add_child(tileset_tilemap_switch_button);

	// Tilemap editor.
	tilemap_editor = memnew(TileMapEditor);
	tilemap_editor->set_h_size_flags(SIZE_EXPAND_FILL);
	tilemap_editor->set_v_size_flags(SIZE_EXPAND_FILL);
	tilemap_editor->hide();
	add_child(tilemap_editor);

	tilemap_toolbar = tilemap_editor->get_toolbar();
	toolbar->add_child(tilemap_toolbar);

	// Tileset editor.
	tileset_editor = memnew(TileSetEditor);
	tileset_editor->set_h_size_flags(SIZE_EXPAND_FILL);
	tileset_editor->set_v_size_flags(SIZE_EXPAND_FILL);
	tileset_editor->hide();
	add_child(tileset_editor);

	// Initialization.
	_update_switch_button();
	_update_editors();
}

TilesEditor::~TilesEditor() {
}

///////////////////////////////////////////////////////////////

void TilesEditorPlugin::_notification(int p_what) {
}

void TilesEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		tiles_editor_button->show();
		editor_node->make_bottom_panel_item_visible(tiles_editor);
		//get_tree()->connect_compat("idle_frame", tileset_editor, "_on_workspace_process");
	} else {
		editor_node->hide_bottom_panel();
		tiles_editor_button->hide();
		//get_tree()->disconnect_compat("idle_frame", tileset_editor, "_on_workspace_process");
	}
}

void TilesEditorPlugin::edit(Object *p_object) {
	tiles_editor->edit(p_object);
}

bool TilesEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("TileMap") || p_object->is_class("TileSet");
}

TilesEditorPlugin::TilesEditorPlugin(EditorNode *p_node) {
	editor_node = p_node;

	tiles_editor = memnew(TilesEditor(p_node));
	tiles_editor->set_custom_minimum_size(Size2(0, 200) * EDSCALE);
	tiles_editor->hide();

	tiles_editor_button = p_node->add_bottom_panel_item(TTR("Tiles"), tiles_editor);
	tiles_editor_button->hide();
}

TilesEditorPlugin::~TilesEditorPlugin() {
}
