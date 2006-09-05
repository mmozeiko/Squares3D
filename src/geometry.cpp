#include "geometry.h"

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
