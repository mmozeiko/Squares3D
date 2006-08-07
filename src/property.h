#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <Newton.h>
#include "common.h"

class Property
{
public:
    Property(float sF, float kF, float eC, float sC);

    void apply(const NewtonMaterial* material);

private:
    float staticFriction;
    float kineticFriction;
    float elasticityCoefficient;
    float softnessCoefficient;
};

#endif
