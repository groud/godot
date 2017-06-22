#include "state_machine_player.h"

Ref<StateMachine> StateMachinePlayer::get_state_machine() const {
  return state_machine;
}

void StateMachinePlayer::set_state_machine(const Ref<StateMachine> p_state_machine) {
  state_machine = p_state_machine;
}

StateMachinePlayer::Mode StateMachinePlayer::get_processing_mode() {
  return processing_mode;
}

void StateMachinePlayer::set_processing_mode(StateMachinePlayer::Mode p_processing_mode) {
  processing_mode = p_processing_mode;
}

void StateMachinePlayer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_state_machine", "state_machine"), &StateMachinePlayer::set_state_machine);
  ClassDB::bind_method(D_METHOD("get_state_machine"), &StateMachinePlayer::get_state_machine);

  ClassDB::bind_method(D_METHOD("set_processing_mode", "mode"), &StateMachinePlayer::set_processing_mode);
  ClassDB::bind_method(D_METHOD("get_processing_mode"), &StateMachinePlayer::get_processing_mode);

  BIND_CONSTANT(MODE_PROCESS);
  BIND_CONSTANT(MODE_FIXED_PROCESS);
  BIND_CONSTANT(MODE_MANUAL);

  ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "state_machine", PROPERTY_HINT_RESOURCE_TYPE, "StateMachine"), "set_state_machine", "get_state_machine");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "processing_mode", PROPERTY_HINT_ENUM, "Process,Fixed process,Manual"), "set_processing_mode", "get_processing_mode");
}

StateMachinePlayer::StateMachinePlayer() {
  processing_mode = MODE_PROCESS;
}
