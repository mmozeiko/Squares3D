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

Vector getSquareCenter(const Vector& lowerLeft, const Vector& upperRight)
{
    return Vector((upperRight[0] + lowerLeft[0])/2, 0.0f, (upperRight[2] + lowerLeft[2])/2);
}

unsigned int getQuadrant(const Vector& point)
{
    //TODO: switch case?
    if ((point[0] >= 0) && (point[2] >= 0))
    {
        return 1;
    }
    else if ((point[0] <= 0) && (point[2] >= 0))
    {
        return 2;
    }
    else if ((point[0] <= 0) && (point[2] <= 0))
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

Vector findBallAndSquareIntersection(const Vector& position, 
                                     const Vector& velocity, 
                                     const Vector& lowerLeft, 
                                     const Vector& upperRight)
{
    //this function returns the center of square if no intersection is found

    float min0 = lowerLeft[0];
    float min2 = lowerLeft[2];
    float max0 = upperRight[0];
    float max2 = upperRight[2];

    float t1 = 0.0f, t2 = 0.0f, x = 0.0f, z = 0.0f;
    bool intersectsX = false, intersectsZ = false;

    Vector squareCenter = getSquareCenter(lowerLeft, upperRight);
    Vector result(squareCenter), intersectionX, intersectionZ;

    t1 = position[2] / static_cast<float>(velocity[2] + 1e-06);
    
    if (t1 <= 0) 
    {
        x = position[0] - velocity[0] * t1;
        if (((x > min0) && (x < max0)) 
            && ((getQuadrant(position)) != (getQuadrant(squareCenter)))) 
        {
            intersectionX = Vector(x, 0.0f, 0.0f);
            intersectsX = true;
        }
    }

    t2 = position[0] / static_cast<float>(velocity[0] + 1e-06);

    if (t2 <= 0) 
    {
        z = position[2] - velocity[2] * t2;
        if (((z > min2) && (z < max2)) 
            && ((getQuadrant(position)) != (getQuadrant(squareCenter))))
        {
            intersectionZ = Vector(0.0f, 0.0f, z);
            intersectsZ = true;
        }
    }

    if (intersectsX && intersectsZ)
    {
        if (t1 > t2)
        {
            result = intersectionX;
        }
        else { result = intersectionZ; }
    }
    else if (intersectsX) { result = intersectionX; }
    else if (intersectsZ) { result = intersectionZ; }

    return result;
}
