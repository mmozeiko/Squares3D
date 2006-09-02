#include <Newton.h>

#include "property.h"

Property::Property(float sF, float kF, float eC, float sC)
    : staticFriction(sF),
      kineticFriction(kF),
      elasticityCoefficient(eC),
      softnessCoefficient(sC)
{
}

void Property::apply(const NewtonMaterial* material) const
{
    NewtonMaterialSetContactSoftness(material, softnessCoefficient);
    NewtonMaterialSetContactElasticity(material, elasticityCoefficient);
    NewtonMaterialSetContactStaticFrictionCoef(material, staticFriction, 0);
    NewtonMaterialSetContactStaticFrictionCoef(material, staticFriction, 1);
    NewtonMaterialSetContactKineticFrictionCoef(material, kineticFriction, 0);
    NewtonMaterialSetContactKineticFrictionCoef(material, kineticFriction, 1);
}
