#include "fence.h"
#include "common.h"
#include "level.h"
#include "collision.h"
#include "vmath.h"
#include "body.h"

void makeFence(Level* level, const NewtonWorld* newtonWorld)
{
    CollisionSet fencePartsCollisions;
    fencePartsCollisions.insert(level->getCollision("fence"));
    fencePartsCollisions.insert(level->getCollision("fenceClip1"));
    //fencePartsCollisions.insert(level->getCollision("fenceClip2"));
    fencePartsCollisions.insert(level->getCollision("fenceTop"));

    Collision* heightMap = level->getCollision("level");

    const float fenceWidth = 0.3f;
    const float fenceHeight = 1.0f;
    const float fenceSpacing = fenceWidth + fenceWidth / 1.5f;

    for (size_t fencesVectorIdx = 0; fencesVectorIdx < level->m_fences.size(); fencesVectorIdx++)
    {
        const vector<Vector>& fence = level->m_fences[fencesVectorIdx];
        for (size_t i = 0; i < fence.size() - 1; i++)
        {    
            Vector startPoint = fence[i];
            Vector endPoint = fence[i + 1];
            Vector delta(endPoint - startPoint);

            Vector direction = delta;
            Vector rotation(0, - direction.getRotationY(), 0);
            
            float howMany = delta.magnitude() / fenceSpacing;
            delta /= howMany;
            for (int j = 0; j < howMany; j++)
            {
                string bodyID = "fence" + cast<string>(fencesVectorIdx) + "_" + cast<string>(i) + "_" + cast<string>(j);
                Body* body = new Body(bodyID, level, &fencePartsCollisions);
                Vector position = Vector(startPoint + delta * static_cast<float>(j));
                position.y = fenceHeight / 2 + heightMap->getHeight(position.x, position.z) + 0.05f;
                body->setTransform(position, rotation);
                NewtonWorldFreezeBody(newtonWorld, body->m_newtonBody);
                NewtonBodySetAutoFreeze(body->m_newtonBody, 1);
                level->m_bodies[bodyID] = body;
            }
        }
    }
}

