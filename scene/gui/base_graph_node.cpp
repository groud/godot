/*************************************************************************/
/*  graph_node.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "graph_node.h"

void BaseGraphNode::set_resizeable(bool p_enable) {

	resizeable = p_enable;
	update();
}

bool BaseGraphNode::is_resizeable() const {

	return resizeable;
}

void BaseGraphNode::_notification(int p_what) {
  if (p_what == NOTIFICATION_DRAW) {
		Ref<Texture> resizer = get_icon("resizer");

		if (resizeable) {
			draw_texture(resizer, get_size() - resizer->get_size());
		}
  }
}

void BaseGraphNode::_gui_input(const Ref<InputEvent> &p_ev) {

	Ref<InputEventMouseButton> mb = p_ev;
	if (mb.is_valid()) {
    if (mb->is_pressed() && mb->get_button_index() == BUTTON_LEFT) {
			Ref<Texture> resizer = get_icon("resizer");
			Vector2 mpos = Vector2(mb->get_position().x, mb->get_position().y);

			if (resizeable && mpos.x > get_size().x - resizer->get_width() && mpos.y > get_size().y - resizer->get_height()) {
				resizing = true;
				resizing_from = mpos;
				resizing_from_size = get_size();
				accept_event();
				return;
			}
    }
		if (!mb->is_pressed() && mb->get_button_index() == BUTTON_LEFT) {
			resizing = false;
		}
  }

  Ref<InputEventMouseMotion> mm = p_ev;
	if (resizing && mm.is_valid()) {
		Vector2 mpos = mm->get_position();

		Vector2 diff = mpos - resizing_from;

		emit_signal("resize_request", resizing_from_size + diff);
	}
}

bool BaseGraphNode::has_point(const Point2 &p_point) const {
		Ref<Texture> resizer = get_icon("resizer");

		if (Rect2(get_size() - resizer->get_size(), resizer->get_size()).has_point(p_point)) {
			return true;
		}

		return BaseGraphNode::has_point(p_point);
}

void BaseGraphNode::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_resizeable", "resizeable"), &BaseGraphNode::set_resizeable);
	ClassDB::bind_method(D_METHOD("is_resizeable"), &BaseGraphNode::is_resizeable);

  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "resizeable"), "set_resizeable", "is_resizeable");

  ADD_SIGNAL(MethodInfo("resize_request", PropertyInfo(Variant::VECTOR2, "new_minsize")));
}

BaseGraphNode::BaseGraphNode() {
	resizeable = false;
	resizing = false;
}
