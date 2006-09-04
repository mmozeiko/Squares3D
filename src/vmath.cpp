#include "vmath.h"

const Vector Vector::Zero(0.0f, 0.0f, 0.0f, 0.0f);
const Vector Vector::One(1.0f, 1.0f, 1.0f, 1.0f);
const Vector Vector::X(1.0f, 0.0f, 0.0f, 0.0f);
const Vector Vector::Y(0.0f, 1.0f, 0.0f, 0.0f);
const Vector Vector::Z(0.0f, 0.0f, 1.0f, 0.0f);

//    |-----|max
//    |     |
// min|-----|
bool isPointInRectangle(const Vector& position, const Vector& lowerLeft, const Vector& upperRight)
{
  if ((position[0] >= lowerLeft[0]) 
      && (position[0] <= upperRight[0])
      && (position[2] >= lowerLeft[2]) 
      && (position[2] <= upperRight[2]))
  {
      return true;
  }
  return false;
}
