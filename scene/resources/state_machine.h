#ifndef FINITESTATEMACHINE_H
#define FINITESTATEMACHINE_H

#include "scene/main/node.h"
#include "core/dictionary.h"
#include "core/array.h"
#include "core/vector.h"

/*class StateMachineTransition: public Resource {
  GDCLASS(StateMachineTransition, Resource);
protected:
  static void _bind_methods();

private:
  Ref<StateMachineState> * target;

  struct Condition {
    NodePath path;
    StringName method;
    Vector<Variant> params;
  };
  Vector<Condition> conditions;

public:
  void _set_default_state(int p_state);
  int _get_default_state() const;
};*/


class StateMachine: public Resource {
  GDCLASS(StateMachine, Resource);
  RES_BASE_EXTENSION("fsm");

protected:
  static void _bind_methods();

private:
  struct State {
    StringName name;
  };

  struct StateData {
    Point2 position;
    State state;
  };

  Vector<StateData> states;
//  Map <int, StateMachineTransition> transitions;

public:
  /*int add_state();
  void remove_state(int p_state);
  int get_state_count() const;

  void add_transition();
  void transition(int p_state, int p_transition);

  void set_default_state(int p_state);
  int get_default_state() const;
*/
  int add_state(const StringName &name, const Point2 &p_position);

  int get_state_count();

  const StringName state_get_name(int p_state);
  void state_set_name(int p_state, const StringName &name);

  Vector2 state_get_position(int p_state);
  void state_set_position(int p_state, Vector2 position);


  Array get_states();
  void set_states(const Array &p_states);

  StateMachine();
  ~StateMachine();
};

#endif
