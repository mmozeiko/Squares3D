//****************************************************************************//
// saver.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_SAVER_H
#define CAL_SAVER_H

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/global.h"
#include "cal3d/vector.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalCoreModel;
class CalCoreSkeleton;
class CalCoreBone;
class CalCoreAnimation;
class CalCoreMesh;
class CalCoreSubmesh;
class CalCoreMaterial;
class CalCoreKeyframe;
class CalCoreTrack;

//****************************************************************************//
// Class declaration                                                          //
//****************************************************************************//

 /*****************************************************************************/
/** The saver class.
  *****************************************************************************/

class CAL3D_API CalSaver
{
public:
  static bool saveCoreAnimation(std::ostream& outputStream, CalCoreAnimation *pCoreAnimation);
  static bool saveCoreMaterial(std::ostream& outputStream, CalCoreMaterial *pCoreMaterial);
  static bool saveCoreMesh(std::ostream& outputStream, CalCoreMesh *pCoreMesh);
  static bool saveCoreSkeleton(std::ostream& outputStream, CalCoreSkeleton *pCoreSkeleton);

protected:
  static bool saveCoreBones(std::ostream& outputStream, CalCoreBone *pCoreBone);
  static bool saveCoreKeyframe(std::ostream& outputStream, CalCoreKeyframe *pCoreKeyframe);
  static bool saveCoreSubmesh(std::ostream& outputStream, CalCoreSubmesh *pCoreSubmesh);
  static bool saveCoreTrack(std::ostream& outputStream, CalCoreTrack *pCoreTrack);
};

#endif

//****************************************************************************//
