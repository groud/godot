#include "state_machine.h"
#if 0

bool StateMachine::_set(const StringName &p_name, const Variant &p_value) {
/*  String name = p_name;

  if (name == "default")
    set_default_(p_value);

  else if (name.begins_with("states/")) {
    int state = name.get_slicec('/', 1).to_int();
    String what = name.get_slicec('/', 2);

    if(what == "name") {
      add_state();
      state_set_name(p_value);
      return true;
    }

  } else if (name.begins_with("transitions/")) {
    int transition = name.get_slicec('/', 1).to_int();
    String what = name.get_slicec('/', 2);

    if(what == "from") {
      add_transition();
      transitions_set_from(transition, p_value);
      return true;
    } else if (what == "to") {
      transitions_set_to(transition, p_value);
      return true;
    }
  }
*/
  return false;
}
bool StateMachine::_get(const StringName &p_name, Variant &r_ret) const {

}
void StateMachine::_get_property_list(List<PropertyInfo> *p_list) const {
/*
	p_list->push_back(PropertyInfo(Variant::REAL, "length", PROPERTY_HINT_RANGE, "0.001,99999,0.001"));
	p_list->push_back(PropertyInfo(Variant::BOOL, "loop"));
	p_list->push_back(PropertyInfo(Variant::REAL, "step", PROPERTY_HINT_RANGE, "0,4096,0.001"));

	for (int i = 0; i < tracks.size(); i++) {

		p_list->push_back(PropertyInfo(Variant::STRING, "tracks/" + itos(i) + "/type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		p_list->push_back(PropertyInfo(Variant::NODE_PATH, "tracks/" + itos(i) + "/path", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		p_list->push_back(PropertyInfo(Variant::INT, "tracks/" + itos(i) + "/interp", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		p_list->push_back(PropertyInfo(Variant::BOOL, "tracks/" + itos(i) + "/loop_wrap", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		p_list->push_back(PropertyInfo(Variant::BOOL, "tracks/" + itos(i) + "/imported", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
		p_list->push_back(PropertyInfo(Variant::ARRAY, "tracks/" + itos(i) + "/keys", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
	}*/
}
int StateMachine::add_state() {
  states.append(memnew(State));
  return states.size()-1;
}
void StateMachine::remove_state(int p_state) {

}

int get_state_count() const {

}

void set_default_state(int p_state) {

}

int get_default_state() const {
  return 0;
}
#endif

int StateMachine::add_state(const StringName &p_name, const Vector2 &p_position) {
    StateData state_data;
    state_data.position = p_position;
    state_data.state.name = p_name;
    //state_data.state->set_name("toto");
    states.push_back(state_data);
    return states.size()-1;
}

const StringName StateMachine::state_get_name(int p_state) {
  return states[p_state].state.name;
}

void StateMachine::state_set_name(int p_state, const StringName &name) {
  states[p_state].state.name = name;
}

Vector2 StateMachine::state_get_position(int p_state) {
  return states[p_state].position;
}

void StateMachine::state_set_position(int p_state, Vector2 position) {
  states[p_state].position = position;
}

Array StateMachine::get_states() {
  Array array;
   for(int i=0; i<states.size(); i++) {
    Dictionary state_d;
    state_d["name"]     = states[i].state.name;
    state_d["position"] = states[i].position;
    array.push_back(state_d);
  }
  return array;
}

void StateMachine::set_states(const Array &p_states) {
  states.clear();
  for (int i=0; i < p_states.size(); i++) {
    Dictionary d = p_states[i];
    add_state(d["name"], d["position"]);
  }
}

void StateMachine::_bind_methods() {
 // ClassDB::bind_method(D_METHOD("add_state", "state", "position"), &StateMachine::add_state);

  ClassDB::bind_method(D_METHOD("set_states", "states"), &StateMachine::set_states);
  ClassDB::bind_method(D_METHOD("get_states"), &StateMachine::get_states);

 // ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "states", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_states", "get_states");
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "states"), "set_states", "get_states");
}

StateMachine::StateMachine() {
  ERR_PRINT("Constructor must not be called!");
}

StateMachine::~StateMachine() {
}
