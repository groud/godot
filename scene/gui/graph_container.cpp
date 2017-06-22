#include "graph_container.h"

bool GraphContainer::_drag_move_test(const Point2 &pos) const {
  return true;
}

int GraphContainer::_drag_hit_test(const Point2 &pos) const {
  int drag_type = DRAG_NONE;

  if (resizeable) {
    int scaleborder_size = get_constant("scaleborder_size", "GraphContainer");

    Rect2 rect = get_rect();

    if (pos.y < (scaleborder_size))
      drag_type = DRAG_RESIZE_TOP;
    else if (pos.y >= (rect.size.height - scaleborder_size))
      drag_type = DRAG_RESIZE_BOTTOM;
    if (pos.x < scaleborder_size)
      drag_type |= DRAG_RESIZE_LEFT;
    else if (pos.x >= (rect.size.width - scaleborder_size))
      drag_type |= DRAG_RESIZE_RIGHT;
  }

  if (movable) {
    if (drag_type == DRAG_NONE && _drag_move_test(pos))
      drag_type = DRAG_MOVE;
  }

  return drag_type;
}

bool GraphContainer::has_point(const Point2 &p_point) const {
  Rect2 r(Point2(), get_size());

  // Inflate by the resizeable border thickness.
  if (resizeable) {
    int scaleborder_size = get_constant("scaleborder_size", "GraphContainer");
    r.position.x -= scaleborder_size;
    r.size.width += scaleborder_size * 2;
    r.position.y -= scaleborder_size;
    r.size.height += scaleborder_size * 2;
  }

  return r.has_point(p_point);
}

Size2 GraphContainer::get_minimum_size() const {
  Size2 ms;
  for (int i = 0; i < get_child_count(); i++) {

    Control *c = get_child(i)->cast_to<Control>();
    if (!c)
      continue;
    if (c->is_set_as_toplevel())
      continue;
    if (!c->is_visible())
      continue;
    Size2 minsize = c->get_combined_minimum_size();
    ms.width = MAX(ms.width, minsize.width);
    ms.height = MAX(ms.height, minsize.height);
  }

  return ms;
}

void GraphContainer::set_resizeable(bool p_enable) {

  resizeable = p_enable;
  update();
}

bool GraphContainer::is_resizeable() const {

  return resizeable;
}

void GraphContainer::set_movable(bool p_enable) {

  movable = p_enable;
  update();
}

bool GraphContainer::is_movable() const {

  return movable;
}

/*void GraphContainer::set_offset(const Vector2 &p_offset) {

	offset = p_offset;
	emit_signal("offset_changed");
	update();
}

Vector2 GraphContainer::get_offset() const {

	return offset;
}*/
void GraphContainer::_notification(int p_what) {
  switch (p_what) {
    case NOTIFICATION_DRAW:
      {
        RID canvas = get_canvas_item();

        // Draw the background.
        Ref<StyleBox> panel = get_stylebox("panel", "GraphContainer");
        Size2 size = get_size();
        panel->draw(canvas, Rect2(0, 0, size.x, size.y));
      } break;

    case NOTIFICATION_MOUSE_EXIT:
      {
        // Reset the mouse cursor when leaving the resizeable window border.
        if (resizeable && !drag_type) {
          if (get_default_cursor_shape() != CURSOR_ARROW)
            set_default_cursor_shape(CURSOR_ARROW);
        }
      } break;

    case NOTIFICATION_SORT_CHILDREN:
      {
        for (int i = 0; i < get_child_count(); i++) {

          Control *c = get_child(i)->cast_to<Control>();
          if (!c)
            continue;
          if (c->is_set_as_toplevel())
            continue;

          fit_child_in_rect(c, Rect2(Point2(0,0), get_size()));
        }
      } break;
  }
}


void GraphContainer::_gui_input(const Ref<InputEvent> &p_event) {
  Ref<InputEventMouseButton> mb = p_event;

  if (mb.is_valid() && mb->get_button_index() == BUTTON_LEFT) {

    if (mb->is_pressed()) {
      // Begin a possible dragging operation.
      drag_type = _drag_hit_test(Point2(mb->get_position().x, mb->get_position().y));
      if (drag_type != DRAG_NONE)
        drag_offset = get_global_mouse_position() - get_position();
      drag_offset_far = get_position() + get_size() - get_global_mouse_position();
    } else if (drag_type != DRAG_NONE && !mb->is_pressed()) {
      // End a dragging operation.
      drag_type = DRAG_NONE;
    }
  }

  Ref<InputEventMouseMotion> mm = p_event;

  if (mm.is_valid()) {

    if (drag_type == DRAG_NONE) {
      // Update the cursor while moving along the borders.
      CursorShape cursor = CURSOR_ARROW;
      if (resizeable) {
        int preview_drag_type = _drag_hit_test(Point2(mm->get_position().x, mm->get_position().y));
        switch (preview_drag_type) {
          case DRAG_RESIZE_TOP:
          case DRAG_RESIZE_BOTTOM:
            cursor = CURSOR_VSIZE;
            break;
          case DRAG_RESIZE_LEFT:
          case DRAG_RESIZE_RIGHT:
            cursor = CURSOR_HSIZE;
            break;
          case DRAG_RESIZE_TOP + DRAG_RESIZE_LEFT:
          case DRAG_RESIZE_BOTTOM + DRAG_RESIZE_RIGHT:
            cursor = CURSOR_FDIAGSIZE;
            break;
          case DRAG_RESIZE_TOP + DRAG_RESIZE_RIGHT:
          case DRAG_RESIZE_BOTTOM + DRAG_RESIZE_LEFT:
            cursor = CURSOR_BDIAGSIZE;
            break;
        }
      }
      if (get_cursor_shape() != cursor)
        set_default_cursor_shape(cursor);
    }/* else {

      // Update while in a dragging operation.
      Point2 global_pos = get_global_mouse_position();
      global_pos.y = MAX(global_pos.y, 0); // Ensure title bar stays visible.

      Rect2 rect = get_rect();
      Size2 min_size = get_minimum_size();

      if (drag_type == DRAG_MOVE) {
        rect.position = global_pos - drag_offset;
      } else {
        if (drag_type & DRAG_RESIZE_TOP) {
          int bottom = rect.position.y + rect.size.height;
          int max_y = bottom - min_size.height;
          rect.position.y = MIN(global_pos.y - drag_offset.y, max_y);
          rect.size.height = bottom - rect.position.y;
        } else if (drag_type & DRAG_RESIZE_BOTTOM) {
          rect.size.height = global_pos.y - rect.position.y + drag_offset_far.y;
        }
        if (drag_type & DRAG_RESIZE_LEFT) {
          int right = rect.position.x + rect.size.width;
          int max_x = right - min_size.width;

          rect.position.x = MIN(global_pos.x - drag_offset.x, max_x);

          rect.size.width = right - rect.position.x;
        } else if (drag_type & DRAG_RESIZE_RIGHT) {
          rect.size.width = global_pos.x - rect.position.x + drag_offset_far.x;
        }
      }

      set_size(rect.size);
      set_offset(rect.position);
    }*/
  }
}

void GraphContainer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("_gui_input"), &GraphContainer::_gui_input);

  ClassDB::bind_method(D_METHOD("set_resizeable", "resizeable"), &GraphContainer::set_resizeable);
  ClassDB::bind_method(D_METHOD("is_resizeable"), &GraphContainer::is_resizeable);

  ClassDB::bind_method(D_METHOD("set_movable", "movable"), &GraphContainer::set_movable);
  ClassDB::bind_method(D_METHOD("is_movable"), &GraphContainer::is_movable);

  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "resizeable"), "set_resizeable", "is_resizeable");
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "movable"), "set_movable", "is_movable");

  ADD_SIGNAL(MethodInfo("resize_request", PropertyInfo(Variant::VECTOR2, "new_minsize")));
}

GraphContainer::GraphContainer() {
  resizeable = true;
  movable = true;
}
