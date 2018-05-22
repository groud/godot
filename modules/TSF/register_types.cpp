
#include "register_types.h"
#include "class_db.h"
#include "MidiStream.h"
#include "MidiStreamPlayback.h"

void register_TSF_types() {

        ClassDB::register_class<MidiStream>();
        ClassDB::register_class<MidiStreamPlayback>();
}

void unregister_TSF_types() {
   //nothing to do here
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        