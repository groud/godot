#ifndef STATE_MACHINE_EDITOR_H
#define STATE_MACHINE_EDITOR_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/gui/split_container.h"
#include "scene/gui/button.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/button_group.h"
#include "scene/gui/box_container.h"
#include "scene/gui/graph_edit.h"
#include "scene/state_machine_player.h"

class StateMachineEditor : public VBoxContainer {
  GDCLASS(StateMachineEditor, VBoxContainer);

  //Global
  HBoxContainer * toolbar;
  HSplitContainer * hsplit;
  Control * state_machine_editor;
  Label * select_state_machine_warning;


  // Left panel
  VBoxContainer * left_panel;
  ToolButton * pin;
  ToolButton * button_add_state;

  // New state button
  ItemList * list_states;

  // State list

  // Selected node properties
  // Name
  // Transition

  // Right panel (on verra plus tard)
  GraphEdit * graph;
  Vector<GraphNode *> graph_nodes;

  //PopupMenu * state_popup;
  PopupMenu * graph_popup;
  Point2 graph_action_position;

  Ref<StateMachine> state_machine;
private:
  void _add_state(const StringName &name, Point2 position);
  void _add_state_button();
  void _update_states();
  void _update_state_dragged(Vector2 from, Vector2 to, int idx);
  void _request_popup(Point2 mouse_pos);
  void _graph_popup(int p_option);
  void _show_select_node_warning(bool b_show);
  void _editor_draw();

protected:
  static void _bind_methods();
  void _notification(int p_what);

public:
  enum MenuItems {
    MENU_NEW_STATE
  };

  void edit(const Ref<StateMachine> &p_state_machine);

  StateMachineEditor();
};



class StateMachineEditorPlugin : public EditorPlugin {
  GDCLASS(StateMachineEditorPlugin, EditorPlugin);

  StateMachineEditor * state_machine_editor;
  EditorNode * editor;

public:
  virtual void edit(Object *p_node);
  virtual void edited_scene_changed();
  virtual void clear();
  virtual void make_visible(bool visible);
  virtual bool handles(Object *p_object) const ;

  StateMachineEditorPlugin(EditorNode *p_node);
  ~StateMachineEditorPlugin();
};
#endif
