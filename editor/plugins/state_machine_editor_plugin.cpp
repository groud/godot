#include "state_machine_editor_plugin.h"

void StateMachineEditor::_update_states() {
  //ERR_FAIL_NULL(state_machine);

  // Clear all
  for(int i=0; i < graph_nodes.size(); i++) {
    memdelete(graph_nodes[i]);
  }
  graph_nodes.clear();
  list_states->clear();

  // Add states
  if(state_machine.is_valid()) {
    for(int idx=0; idx < state_machine->get_states().size(); idx++) {
      GraphNode * gn = memnew(GraphNode);
      const Dictionary &d = state_machine->get_states()[idx];
      const Point2 &pt = d["position"];
      gn->set_offset(pt);
      gn->set_title(d["name"]);
      gn->connect("dragged", this, "_update_state_dragged", varray(idx));
      graph->add_child(gn);
      graph_nodes.push_back(gn);
      list_states->add_item("toto");
    }
  }
}

void StateMachineEditor::_update_state_dragged(Vector2 from, Vector2 to, int idx) {
  state_machine->state_set_position(idx, to);
}

void StateMachineEditor::_request_popup(Point2 mouse_pos) {
  graph_action_position = graph->get_global_transform().affine_inverse().xform(mouse_pos);
  graph_action_position += graph->get_scroll_ofs();
  graph_popup->set_position(mouse_pos);
  graph_popup->popup();
}

void StateMachineEditor::_graph_popup(int p_option) {

  Vector2 ofs = graph_action_position;
  if (graph->is_using_snap()) {
    int snap = graph->get_snap();
    ofs = ofs.snapped(Vector2(snap, snap));
  }
  ofs /= EDSCALE;

  switch(p_option) {
    case MENU_NEW_STATE:
      _add_state("tata", ofs);
      break;
  };
}

void StateMachineEditor::_add_state(const StringName &name, Point2 position) {
  //Add a state to the state machine()
  int id = state_machine->add_state(name, position);
  _update_states();
  graph_nodes[id]->set_selected(true);
}

void StateMachineEditor::_add_state_button() {
  // ERR_FAIL_NULL(state_machine);

  Vector2 ofs = graph->get_scroll_ofs() + graph->get_size() * 0.5;
  if (graph->is_using_snap()) {
    int snap = graph->get_snap();
    ofs = ofs.snapped(Vector2(snap, snap));
  }
  ofs /= EDSCALE;

  _add_state("toto", ofs);
}


void StateMachineEditor::_show_select_node_warning(bool b_show) {
  select_state_machine_warning->set_visible(b_show);
}

void StateMachineEditor::edit(const Ref<StateMachine> &p_state_machine) {
  if(pin->is_pressed())
    return; //ignore, pinned
  state_machine = p_state_machine;

  if (p_state_machine.is_valid()) {
    _show_select_node_warning(false);
  } else {
    _show_select_node_warning(true);
  }
  _update_states();
}

void StateMachineEditor::_editor_draw() {
  if(!state_machine.is_valid()) {
    button_add_state->set_disabled(true);
    return;
  }
  button_add_state->set_disabled(false);
}

void StateMachineEditor::_notification(int p_what) {
  if (p_what == NOTIFICATION_ENTER_TREE) {
    pin->set_icon(get_icon("Pin", "EditorIcons"));

  } else if (p_what == NOTIFICATION_DRAW) {
    _editor_draw();
  }
}
void StateMachineEditor::_bind_methods() {
  ClassDB::bind_method(D_METHOD("_add_state_button"), &StateMachineEditor::_add_state_button);
  ClassDB::bind_method(D_METHOD("_update_states"), &StateMachineEditor::_add_state_button);
  ClassDB::bind_method(D_METHOD("_update_state_dragged"), &StateMachineEditor::_update_state_dragged);
  ClassDB::bind_method(D_METHOD("_request_popup"), &StateMachineEditor::_request_popup);
  ClassDB::bind_method(D_METHOD("_graph_popup"), &StateMachineEditor::_graph_popup);

  BIND_CONSTANT(MENU_NEW_STATE);
}

StateMachineEditor::StateMachineEditor() {

  // Toolbar
  toolbar = memnew(HBoxContainer);
  add_child(toolbar);

  pin = memnew(ToolButton);
  pin->set_toggle_mode(true);
  toolbar->add_child(pin);

  button_add_state = memnew(ToolButton());
  button_add_state->set_text(TTR("Add state"));
  button_add_state->connect("pressed", this, "_add_state_button");
  button_add_state->set_disabled(false);
  toolbar->add_child(button_add_state);

  // Editor
  state_machine_editor = memnew(Control);
  state_machine_editor->set_v_size_flags(SIZE_EXPAND_FILL);
  state_machine_editor->set_h_size_flags(SIZE_EXPAND_FILL);
  state_machine_editor->set_custom_minimum_size(Size2(0,180));
  add_child(state_machine_editor);

  hsplit = memnew(HSplitContainer);
  hsplit->set_v_size_flags(SIZE_EXPAND_FILL);
  hsplit->set_area_as_parent_rect();
  state_machine_editor->add_child(hsplit);

  select_state_machine_warning = memnew(Label);
  select_state_machine_warning->set_area_as_parent_rect();
  select_state_machine_warning->set_text(TTR("Select a StateMachinePlayer from the Scene Tree to edit its state machine."));
  select_state_machine_warning->set_autowrap(true);
  select_state_machine_warning->set_align(Label::ALIGN_CENTER);
  select_state_machine_warning->set_valign(Label::VALIGN_CENTER);
  state_machine_editor->add_child(select_state_machine_warning);

  left_panel = memnew(VBoxContainer());
  hsplit->add_child(left_panel);

  list_states = memnew(ItemList());
  list_states->set_v_size_flags(SIZE_EXPAND_FILL);
  left_panel->add_child(list_states);

  graph = memnew(GraphEdit());
  graph->connect("popup_request", this, "_request_popup");
  hsplit->add_child(graph);

  /*state_popup = memnew(MenuPopup);
    state_popup->add_item("Delete");
    */
  graph_popup = memnew(PopupMenu);
  graph_popup->add_item(TTR("New state"), MENU_NEW_STATE);
  graph_popup->connect("id_pressed", this, "_graph_popup");
  hsplit->add_child(graph_popup);
}












bool StateMachineEditorPlugin::handles(Object * p_object) const {
  return p_object->is_class("StateMachine")||p_object->is_class("StateMachinePlayer");
}

void StateMachineEditorPlugin::clear() {
  print_line("cleared A !");
  if(state_machine_editor) state_machine_editor->edit(NULL);
 /* state_machine = Ref<StateMachine>();
  _show_select_node_warning(true);*/
}

void StateMachineEditorPlugin::make_visible(bool visible) {
  if(visible) {
    editor->make_bottom_panel_item_visible(state_machine_editor);
  }
}

void StateMachineEditorPlugin::edited_scene_changed() {
  print_line("cleared B !");
  if(state_machine_editor) state_machine_editor->edit(NULL);
 /* state_machine = Ref<StateMachine>();
  _show_select_node_warning(true);*/
}

void StateMachineEditorPlugin::edit(Object * p_object) {
  //state_machine_editor->set_undo_redo(&get_undo_redo());
  if(!p_object)
    return;

  if (p_object->cast_to<StateMachine>()) {
    state_machine_editor->edit(p_object->cast_to<StateMachine>());
  } else if (p_object->cast_to<StateMachinePlayer>()) {
    state_machine_editor->edit(p_object->cast_to<StateMachinePlayer>()->get_state_machine());
  } else {
    state_machine_editor->edit(NULL);
  }
}

StateMachineEditorPlugin::StateMachineEditorPlugin(EditorNode *p_node) {
  editor = p_node;
  state_machine_editor = memnew(StateMachineEditor());
  //state_machine_editor->set_undo_redo(editor->get_undo_redo());

  editor->add_bottom_panel_item(TTR("State machine"), state_machine_editor);
  //state_machine_editor->hide();
}

StateMachineEditorPlugin::~StateMachineEditorPlugin() {

}

