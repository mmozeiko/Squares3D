#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "vmath.h"
#include "common.h"

#include <GL/glfw.h>

class Game;

typedef vector<Vector> Face;

class Video
{
public:
    Video(const Game* game);
    ~Video();

    void renderCube(float size = 1.0f) const;
    void renderFace(const Face& face, vector<vector<int>>&) const;
    void renderSphere(float radius = 1.0f) const;
    void renderWireSphere(float radius = 1.0f) const;
    void renderAxes(float size = 5.0f) const;

    void beginObject(const Matrix& matrix) const;
    void endObject() const;

    unsigned int loadTexture(const string& name) const;

private:
    const Game* m_game;
};

#endif
