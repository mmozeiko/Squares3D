try:
    import psyco
    psyco.full()
except ImportError:
    print 'Psyco not installed, the program will just run slower'

import os
from struct import pack
from math import sqrt
from operator import *

DATA_PATH = os.path.join("..", "bin", "data", "heightmaps")

hmap = []
size = 0.0
size2 = 0.0
X = 0.0
Y = 0.0

def H(x, z):
  return hmap[int((z+size2)*Y/size-0.5)][int((x+size2)*X/size-0.5)]

def pointInCircle(px, py, rx, ry, r):
  x = px-ry
  y = py-ry
  return r*r >= x*x+y*y

class Tri:
  def __init__(self, a, b, c):
    self.a = a
    self.b = b
    self.c = c
   
  def __eq__(self, z):
    return self.a==z.a and self.b==z.b and self.c==z.c or \
           self.a==z.a and self.b==z.c and self.c==z.b or \
           self.a==z.b and self.b==z.a and self.c==z.c or \
           self.a==z.b and self.b==z.c and self.c==z.a or \
           self.a==z.c and self.b==z.a and self.c==z.b or \
           self.a==z.c and self.b==z.b and self.c==z.a

  def __hash__(self):
    return hash(self.a) ^ hash(self.b) ^ hash(self.c)

def tri(aa, bb, cc, v):
  vA = v[aa]
  vB = v[bb]
  vC = v[cc]

  vAB = [a-b for a,b in zip(vB, vA)]
  vAC = [a-b for a,b in zip(vC, vA)]

  if vAB[2]*vAC[0] - vAB[0]*vAC[2] > 0:
    return Tri(aa,bb,cc)
  return Tri(aa,cc,bb)
           
def process(input, output):
  global X, Y, size, size2, hmap

  f = file(input, "rb")
  data = f.read()
  f.close()
  if ord(data[2]) != 3:
    print "Unsupported TGA format!"
    return
  for offset in [1,3,4,5,6,7,8,9,10,11]:
    if ord(data[offset]) != 0:
      print "Unsupported TGA format!"
      return
  
  X = ord(data[12]) + ord(data[13])*256
  Y = ord(data[14]) + ord(data[15])*256
  bpp = ord(data[16])
  if bpp != 8: 
    print "Unsupported TGA format!"
    return

  idx = 18 + ord(data[0])

  hmap = []
  for y in xrange(Y):
    row = []
    for x in xrange(X):
      row.append((ord(data[idx + y*X + x])-128.0)/20.0)
    hmap.append(row)

  size = 40.0
  size2 = size/2.0

  vertices = []
  triangles = set([])
  normals = []
  uv = []

  maxVertex = 50
  maxErr = 0.01

  err = 99999.999999

  vertices += [(-size2, H(-size2, -size2), -size2, 1.0), 
               (-size2, H(-size2, size2), size2, 1.0),
               (size2, H(size2, size2), size2, 1.0),
               (size2, H(size2, -size2), -size2, 1.0)]

  triangles.add(Tri(0, 1, 3))
  triangles.add(Tri(1, 2, 3))
  biTri = {}
  biTri[(0, 1)] = [3]
  biTri[(1, 3)] = [0]
  biTri[(3, 0)] = [1]
  biTri[(1, 2)] = [3]
  biTri[(2, 3)] = [1]
  biTri[(3, 1)] = [2]

  while err > maxErr and len(vertices) < maxVertex:
    
    # find triangle with max error
    maxE = -1.0
    maxP = None
    maxT = None
    for t in triangles:

      v1 = vertices[t.a]
      v2 = vertices[t.b]
      v3 = vertices[t.c]

      vAB = [a-b for a,b in zip(v2, v1)]
      vBC = [a-b for a,b in zip(v3, v2)]
      vCA = [a-b for a,b in zip(v1, v3)]

      first = [a-b for a,b in zip(v2,v1)]
      second = [a-b for a,b in zip(v3,v1)]

      N = [first[1]*second[2] - first[2]*second[1], 
           first[2]*second[0] - first[0]*second[2], 
           first[0]*second[1] - first[1]*second[0]]

      D = -sum(map(mul, v1[:3], N))

      pz = min(v1[2], v2[2], v3[2])
      while pz<max(v1[2], v2[2], v3[2]):
        px = min(v1[0], v2[0], v3[0])
        while px<max(v1[0], v2[0], v3[0]):
          
          v0 = (px, H(px, pz), pz, 1.0)

          vAP = [a-b for a,b in zip(v0, v1)]
          vBP = [a-b for a,b in zip(v0, v2)]
          vCP = [a-b for a,b in zip(v0, v3)]

          b1 = vAB[2]*vAP[0] - vAB[0]*vAP[2]
          b2 = vBC[2]*vBP[0] - vBC[0]*vBP[2]
          b3 = vCA[2]*vCP[0] - vCA[0]*vCP[2]

          if b1>0.0 and b2>0.0 and b3>0.0:

            err = abs(v0[1] - -(N[0]*v0[0] + N[2]*v0[2]+D)/N[1] )

            if err > maxE:
              maxE = err
              maxP = v0
              maxT = t

          px += size/X
        pz += size/Y

    err = maxE

    print err, maxP

    # insert point s
    triangles.remove(maxT)
    i = len(vertices)
    vertices.append(maxP)
   
    tris = [tri(maxT.a, maxT.b, i, vertices), tri(maxT.b, maxT.c, i, vertices), tri(maxT.c, maxT.a, i, vertices)]
    for t in tris:
      triangles.add(t)

      key = (min(t.a, t.b), max(t.a, t.b))
      if key in biTri:
        #check for circle
        for j in biTri[key]:
          if i==j: continue

          Ax = vertices[t.a][0]
          Ay = vertices[t.a][2]
          Bx = vertices[t.b][0]
          By = vertices[t.b][2]
          Cx = vertices[t.c][0]
          Cy = vertices[t.c][2]
          AA = sqrt(Ax*Ax + Ay*Ay)
          BB = sqrt(Bx*Bx + By*By)
          CC = sqrt(Cx*Cx + Cy*Cy)
          Sx = 0.5 * (AA*By + CC*Ay + BB*Cy - CC*By - BB*Cy - CC*Ay)
          Sy = 0.5 * (Ax*BB + AA*Bx + CC*Ax - BB*Cx - CC*Bx - AA*Cx)
          a = Ax*By + Ay*Bx + Cy*Ax - Cx*By - Bx*Cy - Ay*Cx
          b = Ax*By*CC + Bx*Ay*CC + Cy*BB*Ax - AA*Cx*Cy - BB*Ay*Cx - Bx*Cy*AA

          Sx /= a
          Sy /= a
          r = sqrt(b/a + (Sx*Sx+Sy*Sy)/(a*a))

          if pointInCircle(maxP[0], maxP[2], Sx, Sy, r):
            tris
      else:
        biTri[key] = [t.c]

  print triangles
  out = file(output, "wb")

  out.write(pack(*(["4B"] + [ord(x) for x in ["H", "M", "A", "P"]])))

  out.write(pack(*(["ii"] + [len(vertices), len(triangles)])))

  #vertices
  for v in vertices:
    out.write(pack(*(["ffff"] + list(v))))

  #normals
  for i in xrange(len(vertices)):
    out.write(pack(*(["ffff"] + [0.0, 1.0, 0.0, 1.0])))
    
  #uv
  for v in vertices:
    out.write(pack(*(["ff"] + [v[0]*2, v[2]*2])))

  #indices
  for i in triangles:
    out.write(pack(*(["H"*3] + list(i))))
  
  out.close()

for root, dir, files in os.walk(DATA_PATH):
  for f in files:
    name = os.path.join(root, f)
    if name[-4:] == ".tga":
      print name
      process(name, name[:-4] + ".hmap")
