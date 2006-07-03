//****************************************************************************//
// saver.cpp                                                                  //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/saver.h"
#include "cal3d/error.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coremodel.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/corematerial.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coretrack.h"

using namespace cal3d;

 /*****************************************************************************/
/** Saves a core animation instance.
  *
  * This function saves a core animation instance to a file.
  *
  * @param file  The file stream to save the core animation instance to.
  * @param pCoreAnimation A pointer to the core animation instance that should
  *                       be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreAnimation(std::ostream& outputStream, CalCoreAnimation *pCoreAnimation)
{
  // write magic tag
  if(!CalPlatform::writeBytes(outputStream, &Cal::ANIMATION_FILE_MAGIC, sizeof(Cal::ANIMATION_FILE_MAGIC)))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write version info
  if(!CalPlatform::writeInteger(outputStream, Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write the duration of the core animation
  if(!CalPlatform::writeFloat(outputStream, pCoreAnimation->getDuration()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write the number of tracks
  if(!CalPlatform::writeInteger(outputStream, pCoreAnimation->getTrackCount()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // get core track list
  std::list<CalCoreTrack *>& listCoreTrack = pCoreAnimation->getListCoreTrack();

  // write all core bones
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack)
  {
    // save core track
    if(!saveCoreTrack(outputStream, *iteratorCoreTrack))
    {
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core bone instance.
  *
  * This function saves a core bone instance to a file stream.
  *
  * @param file The file stream to save the core bone instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreBone A pointer to the core bone instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreBones(std::ostream& outputStream, CalCoreBone *pCoreBone)
{
  if(!outputStream)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return false;
  }

  // write the name of the bone
  if(!CalPlatform::writeString(outputStream, pCoreBone->getName()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write the translation of the bone
  const CalVector& translation = pCoreBone->getTranslation();
  CalPlatform::writeFloat(outputStream, translation[0]);
  CalPlatform::writeFloat(outputStream, translation[1]);
  CalPlatform::writeFloat(outputStream, translation[2]);

  // write the rotation of the bone
  const CalQuaternion& rotation = pCoreBone->getRotation();
  CalPlatform::writeFloat(outputStream, rotation[0]);
  CalPlatform::writeFloat(outputStream, rotation[1]);
  CalPlatform::writeFloat(outputStream, rotation[2]);
  CalPlatform::writeFloat(outputStream, rotation[3]);

  // write the translation of the bone
  const CalVector& translationBoneSpace = pCoreBone->getTranslationBoneSpace();
  CalPlatform::writeFloat(outputStream, translationBoneSpace[0]);
  CalPlatform::writeFloat(outputStream, translationBoneSpace[1]);
  CalPlatform::writeFloat(outputStream, translationBoneSpace[2]);

  // write the rotation of the bone
  const CalQuaternion& rotationBoneSpace = pCoreBone->getRotationBoneSpace();
  CalPlatform::writeFloat(outputStream, rotationBoneSpace[0]);
  CalPlatform::writeFloat(outputStream, rotationBoneSpace[1]);
  CalPlatform::writeFloat(outputStream, rotationBoneSpace[2]);
  CalPlatform::writeFloat(outputStream, rotationBoneSpace[3]);

  // write the parent bone id
  if(!CalPlatform::writeInteger(outputStream, pCoreBone->getParentId()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // get children list
  std::list<int>& listChildId = pCoreBone->getListChildId();

  // write the number of children
  if(!CalPlatform::writeInteger(outputStream, listChildId.size()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write all children ids
  std::list<int>::iterator iteratorChildId;
  for(iteratorChildId = listChildId.begin(); iteratorChildId != listChildId.end(); ++iteratorChildId)
  {
    // write the child id
    if(!CalPlatform::writeInteger(outputStream, *iteratorChildId))
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core keyframe instance.
  *
  * This function saves a core keyframe instance to a file stream.
  *
  * @param file The file stream to save the core keyframe instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreKeyframe A pointer to the core keyframe instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreKeyframe(std::ostream& outputStream, CalCoreKeyframe *pCoreKeyframe)
{
  if(!outputStream)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return false;
  }

  // write the time of the keyframe
  CalPlatform::writeFloat(outputStream, pCoreKeyframe->getTime());

  // write the translation of the keyframe
  const CalVector& translation = pCoreKeyframe->getTranslation();
  CalPlatform::writeFloat(outputStream, translation[0]);
  CalPlatform::writeFloat(outputStream, translation[1]);
  CalPlatform::writeFloat(outputStream, translation[2]);

  // write the rotation of the keyframe
  const CalQuaternion& rotation = pCoreKeyframe->getRotation();
  CalPlatform::writeFloat(outputStream, rotation[0]);
  CalPlatform::writeFloat(outputStream, rotation[1]);
  CalPlatform::writeFloat(outputStream, rotation[2]);
  CalPlatform::writeFloat(outputStream, rotation[3]);

  // check if an error happend
  if(!outputStream)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core material instance.
  *
  * This function saves a core material instance to a file.
  *
  * @param file The file stream to save the core material instance to.
  * @param pCoreMaterial A pointer to the core material instance that should
  *                      be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreMaterial(std::ostream& outputStream, CalCoreMaterial *pCoreMaterial)
{
  // write magic tag
  if(!CalPlatform::writeBytes(outputStream, &Cal::MATERIAL_FILE_MAGIC, sizeof(Cal::MATERIAL_FILE_MAGIC)))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write version info
  if(!CalPlatform::writeInteger(outputStream, Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write the ambient color
  CalCoreMaterial::Color ambientColor;
  ambientColor = pCoreMaterial->getAmbientColor();
  CalPlatform::writeBytes(outputStream, &ambientColor, sizeof(ambientColor));

  // write the diffuse color
  CalCoreMaterial::Color diffusetColor;
  diffusetColor = pCoreMaterial->getDiffuseColor();
  CalPlatform::writeBytes(outputStream, &diffusetColor, sizeof(diffusetColor));

  // write the specular color
  CalCoreMaterial::Color specularColor;
  specularColor = pCoreMaterial->getSpecularColor();
  CalPlatform::writeBytes(outputStream, &specularColor, sizeof(specularColor));

  // write the shininess factor
  CalPlatform::writeFloat(outputStream, pCoreMaterial->getShininess());

  // check if an error happend
  if(!outputStream)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // get the map vector
  std::vector<CalCoreMaterial::Map>& vectorMap = pCoreMaterial->getVectorMap();

  // write the number of maps
  if(!CalPlatform::writeInteger(outputStream, vectorMap.size()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write all maps
  int mapId;
  for(mapId = 0; mapId < (int)vectorMap.size(); ++mapId)
  {
    CalCoreMaterial::Map& map = vectorMap[mapId];

    // write the filename of the map
    if(!CalPlatform::writeString(outputStream, map.strFilename))
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core mesh instance.
  *
  * This function saves a core mesh instance to a file.
  *
  * @param strFilename The name of the file to save the core mesh instance to.
  * @param pCoreMesh A pointer to the core mesh instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreMesh(std::ostream& outputStream, CalCoreMesh *pCoreMesh)
{
  // write magic tag
  if(!CalPlatform::writeBytes(outputStream, &Cal::MESH_FILE_MAGIC, sizeof(Cal::MESH_FILE_MAGIC)))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write version info
  if(!CalPlatform::writeInteger(outputStream, Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // get the submesh vector
  std::vector<CalCoreSubmesh *>& vectorCoreSubmesh = pCoreMesh->getVectorCoreSubmesh();

  // write the number of submeshes
  if(!CalPlatform::writeInteger(outputStream, vectorCoreSubmesh.size()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write all core submeshes
  int submeshId;
  for(submeshId = 0; submeshId < (int)vectorCoreSubmesh.size(); ++submeshId)
  {
    // write the core submesh
    if(!saveCoreSubmesh(outputStream, vectorCoreSubmesh[submeshId]))
    {
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core skeleton instance.
  *
  * This function saves a core skeleton instance to a file.
  *
  * @param strFilename The name of the file to save the core skeleton instance
  *                    to.
  * @param pCoreSkeleton A pointer to the core skeleton instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreSkeleton(std::ostream& outputStream, CalCoreSkeleton *pCoreSkeleton)
{
  // write magic tag
  if(!CalPlatform::writeBytes(outputStream, &Cal::SKELETON_FILE_MAGIC, sizeof(Cal::SKELETON_FILE_MAGIC)))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write version info
  if(!CalPlatform::writeInteger(outputStream, Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write the number of bones
  if(!CalPlatform::writeInteger(outputStream, pCoreSkeleton->getVectorCoreBone().size()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write all core bones
  int boneId;
  for(boneId = 0; boneId < (int)pCoreSkeleton->getVectorCoreBone().size(); ++boneId)
  {
    // write the core bone
    if(!saveCoreBones(outputStream, pCoreSkeleton->getCoreBone(boneId)))
    {
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core submesh instance.
  *
  * This function saves a core submesh instance to a file stream.
  *
  * @param file The file stream to save the core submesh instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreSubmesh A pointer to the core submesh instance that should be
  *                     saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreSubmesh(std::ostream& outputStream, CalCoreSubmesh *pCoreSubmesh)
{
  if(!outputStream)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return false;
  }

  // write the core material thread id
  if(!CalPlatform::writeInteger(outputStream, pCoreSubmesh->getCoreMaterialThreadId()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // get the vertex, face, physical property and spring vector
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pCoreSubmesh->getVectorVertex();
  std::vector<CalCoreSubmesh::Face>& vectorFace = pCoreSubmesh->getVectorFace();
  std::vector<CalCoreSubmesh::PhysicalProperty>& vectorPhysicalProperty = pCoreSubmesh->getVectorPhysicalProperty();
  std::vector<CalCoreSubmesh::Spring>& vectorSpring = pCoreSubmesh->getVectorSpring();

  // write the number of vertices, faces, level-of-details and springs
  CalPlatform::writeInteger(outputStream, vectorVertex.size());
  CalPlatform::writeInteger(outputStream, vectorFace.size());
  CalPlatform::writeInteger(outputStream, pCoreSubmesh->getLodCount());
  CalPlatform::writeInteger(outputStream, pCoreSubmesh->getSpringCount());

  // get the texture coordinate vector vector
  std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& vectorvectorTextureCoordinate = pCoreSubmesh->getVectorVectorTextureCoordinate();

  // write the number of texture coordinates per vertex
  CalPlatform::writeInteger(outputStream, vectorvectorTextureCoordinate.size());

  // check if an error happend
  if(!outputStream)
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write all vertices
  int vertexId;
  for(vertexId = 0; vertexId < (int)vectorVertex.size(); ++vertexId)
  {
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];

    // write the vertex data
    CalPlatform::writeFloat(outputStream, vertex.position.x);
    CalPlatform::writeFloat(outputStream, vertex.position.y);
    CalPlatform::writeFloat(outputStream, vertex.position.z);
    CalPlatform::writeFloat(outputStream, vertex.normal.x);
    CalPlatform::writeFloat(outputStream, vertex.normal.y);
    CalPlatform::writeFloat(outputStream, vertex.normal.z);
    CalPlatform::writeInteger(outputStream, vertex.collapseId);
    CalPlatform::writeInteger(outputStream, vertex.faceCollapseCount);

    // write all texture coordinates of this vertex
    int textureCoordinateId;
    for(textureCoordinateId = 0; textureCoordinateId < (int)vectorvectorTextureCoordinate.size(); ++textureCoordinateId)
    {
      CalCoreSubmesh::TextureCoordinate& textureCoordinate = vectorvectorTextureCoordinate[textureCoordinateId][vertexId];

      // write the influence data
      CalPlatform::writeFloat(outputStream, textureCoordinate.u);
      CalPlatform::writeFloat(outputStream, textureCoordinate.v);

      // check if an error happend
      if(!outputStream)
      {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
        return false;
      }
    }

    // write the number of influences
    if(!CalPlatform::writeInteger(outputStream, vertex.vectorInfluence.size()))
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
      return false;
    }

     // write all influences of this vertex
    int influenceId;
    for(influenceId = 0; influenceId < (int)vertex.vectorInfluence.size(); ++influenceId)
    {
      CalCoreSubmesh::Influence& influence = vertex.vectorInfluence[influenceId];

      // write the influence data
      CalPlatform::writeInteger(outputStream, influence.boneId);
      CalPlatform::writeFloat(outputStream, influence.weight);

      // check if an error happend
      if(!outputStream)
      {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
        return false;
      }
    }

    // save the physical property of the vertex if there are springs in the core submesh
    if(pCoreSubmesh->getSpringCount() > 0)
    {
      // write the physical property of this vertex if there are springs in the core submesh
      CalCoreSubmesh::PhysicalProperty& physicalProperty = vectorPhysicalProperty[vertexId];

      // write the physical property data
      CalPlatform::writeFloat(outputStream, physicalProperty.weight);

      // check if an error happend
      if(!outputStream)
      {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
        return false;
      }
    }
  }

  // write all springs
  int springId;
  for(springId = 0; springId < (int)pCoreSubmesh->getSpringCount(); ++springId)
  {
    CalCoreSubmesh::Spring& spring = vectorSpring[springId];

    // write the spring data
    CalPlatform::writeInteger(outputStream, spring.vertexId[0]);
    CalPlatform::writeInteger(outputStream, spring.vertexId[1]);
    CalPlatform::writeFloat(outputStream, spring.springCoefficient);
    CalPlatform::writeFloat(outputStream, spring.idleLength);

    // check if an error happend
    if(!outputStream)
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
      return false;
    }
  }

  // write all faces
  int faceId;
  for(faceId = 0; faceId < (int)vectorFace.size(); ++faceId)
  {
    CalCoreSubmesh::Face& face = vectorFace[faceId];

    // write the face data
    CalPlatform::writeInteger(outputStream, face.vertexId[0]);
    CalPlatform::writeInteger(outputStream, face.vertexId[1]);
    CalPlatform::writeInteger(outputStream, face.vertexId[2]);

    // check if an error happend
    if(!outputStream)
    {
      CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

 /*****************************************************************************/
/** Saves a core track instance.
  *
  * This function saves a core track instance to a file stream.
  *
  * @param file The file stream to save the core track instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreTrack A pointer to the core track instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreTrack(std::ostream& outputStream, CalCoreTrack *pCoreTrack)
{
  if(!outputStream)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return false;
  }

  // write the bone id
  if(!CalPlatform::writeInteger(outputStream, pCoreTrack->getCoreBoneId()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // write the number of keyframes
  if(!CalPlatform::writeInteger(outputStream, pCoreTrack->getCoreKeyframeCount()))
  {
    CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__);
    return false;
  }

  // save all core keyframes
  for(int i = 0; i < pCoreTrack->getCoreKeyframeCount(); ++i)
  {
    // save the core keyframe
    bool res = saveCoreKeyframe(outputStream, pCoreTrack->getCoreKeyframe(i));

    if (!res) {
      return false;
    }
  }

  return true;
}
 