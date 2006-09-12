#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "common.h"
#include "vmath.h"

//    |-----|max
//    |     |
// min|-----|
bool isPointInRectangle(const Vector& position, 
                        const Vector& lowerLeft, 
                        const Vector& upperRight);

Vector getSquareCenter(const Vector& lowerLeft, const Vector& upperRight);

unsigned int getQuadrant(const Vector& point);

Vector findBallAndSquareIntersection(const Vector& position, 
                                     const Vector& velocity, 
                                     const Vector& lowerLeft, 
                                     const Vector& upperRight);

#endif
