/* summator.h */
#ifndef SUMMATOR_H
#define SUMMATOR_H

#include "reference.h"

class Summator : public Reference {
    GDCLASS(Summator, Reference);

    int count;

protected:
    static void _bind_methods();

public:
    void add(int value);
    void reset();
    int get_total() const;

    Summator();
};

#endif






