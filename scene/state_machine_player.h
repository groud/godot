#ifndef STATE_MACHINE_PLAYER_H
#define STATE_MACHINE_PLAYER_H

#include "scene/main/node.h"
#include "scene/resources/state_machine.h"

class StateMachinePlayer : public Node {
  GDCLASS(StateMachinePlayer, Node);

public:
  enum Mode {
    MODE_PROCESS,
    MODE_FIXED_PROCESS,
    MODE_MANUAL
  };

private:
  Ref<StateMachine> state_machine;
  Mode processing_mode;

protected:
  static void _bind_methods();

public:
  Ref<StateMachine> get_state_machine() const;
  void set_state_machine(const Ref<StateMachine>);

  Mode get_processing_mode();
  void set_processing_mode(Mode p_processing_mode);

  StateMachinePlayer();
};

VARIANT_ENUM_CAST(StateMachinePlayer::Mode);

#endif
