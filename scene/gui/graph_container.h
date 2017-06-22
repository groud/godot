
#ifndef RESIZABLE_CONTAINER_H
#define RESIZABLE_CONTAINER_H

#include "container.h"

class GraphContainer : public Container {
	GDCLASS(GraphContainer, Container);

private:
	enum DRAG_TYPE {
		DRAG_NONE = 0,
		DRAG_MOVE = 1,
		DRAG_RESIZE_TOP = 1 << 1,
		DRAG_RESIZE_RIGHT = 1 << 2,
		DRAG_RESIZE_BOTTOM = 1 << 3,
		DRAG_RESIZE_LEFT = 1 << 4
	};

	int drag_type;
	bool resizeable;
	bool movable;
	Point2 drag_offset;
	Point2 drag_offset_far;

	bool has_point(const Point2 &p_point) const;
	int _drag_hit_test(const Point2 &pos) const;
  virtual bool _drag_move_test(const Point2 &pos) const;

protected:
	virtual void _gui_input(const Ref<InputEvent> &p_ev);
	virtual void _notification(int p_what);
	static void _bind_methods();

public:
	void set_resizeable(bool p_enable);
	bool is_resizeable() const;

	void set_movable(bool p_enable);
	bool is_movable() const;
/*
	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;
*/
  virtual Size2 get_minimum_size() const;

  GraphContainer();
};

#endif
