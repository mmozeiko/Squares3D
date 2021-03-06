try:
    import psyco
    psyco.full()
except ImportError:
    print 'Psyco not installed, the program will just run slower'

import os
from struct import pack
from math import sqrt

DATA_PATH = os.path.join("..", "bin", "data", "heightmaps")

def hasTri((i1, i2, i3), triangles):
  for ii1, ii2, ii3 in triangles:
    if i1==ii1 and i2==ii2 and i3==ii3 or \
       i1==ii1 and i2==ii3 and i3==ii2 or \
       i1==ii2 and i2==ii1 and i3==ii3 or \
       i1==ii2 and i2==ii3 and i3==ii1 or \
       i1==ii3 and i2==ii1 and i3==ii2 or \
       i1==ii3 and i2==ii2 and i3==ii1:
      return True
  return False

def process(input, output):
  print "  Reading input..."

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

  vertices = []
  normals = []
  uv = []

  STEP = 0.25

  size = 40.0
  size2 = size/2.0

  x = -size2

  maxIdx = 0
  maxIdxB = False

  print "  Calculating vertexes & normals..."

  vdict = {}
  idx = 0

  while x < size2:
    ix = int((x + size2) * X / size)
    ix2 = int((x + size2 + STEP) * X / size)
    if ix2 >= X: break

    z = -size2
    while z < size2:
      iz = int((z + size2) * Y / size)
      iz2 = int((z + size2 + STEP) * Y / size)
      if iz2 >= Y: break
            
      y1 = hmap[iz][ix]
      y2 = hmap[iz2][ix]
      #y3 = hmap[iz2][ix2]
      y4 = hmap[iz][ix2]

      v0 = (x, y1, z, 1.0)
      v1 = (x, y2, z+STEP, 1.0)
      #v2 = (x+STEP, y3, z+STEP, 1.0)
      v3 = (x+STEP, y4, z, 1.0)

      a = [p1-p2 for (p1,p2) in zip(v1, v0)]
      b = [p1-p2 for (p1,p2) in zip(v3, v0)]
         
      normal = (a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0])

      s = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2])
      normal = (normal[0]/s, normal[1]/s, normal[2]/s, 1.0)
             
      vertices += v0
      normals += normal
      uv += [x*2.0, z*2.0]

      vdict[(str(x),str(z))] = idx
      idx += 1

      z += STEP
      if not maxIdxB: maxIdx += 1

    if not maxIdxB: maxIdxB = True
    x += STEP


  print "  Calculating triangles..."

  indices = []
  triangles = set()

  for x in xrange(maxIdx-1):
    for z in xrange(maxIdx-1):
      i1 = x*maxIdx+z
      i2 = x*maxIdx+z+1
      i3 = (x+1)*maxIdx+z+1
      i4 = (x+1)*maxIdx+z
            
      if i3 > 65535:
        raise "Too many vertices in heightmap!!"

      indices += [i1, i2, i4, i2, i3, i4]
      triangles.add((i1, i2, i4))
      triangles.add((i2, i3, i4))
    
  print "  Optimizing triangles..."

  print "  Total = ", len(triangles)

  #print "\n".join(map(str, vdict.items()))
  #for t in triangles:
  #  print t[0], ",", t[1], ",", t[2]
  #print "\n".join(map(str, list(triangles)))

  changed = True
  while changed:
    changed = False
    for i1, i2, i3 in triangles:
      v1 = [vertices[4*i1+t] for t in range(3)]
      v2 = [vertices[4*i2+t] for t in range(3)]
      v3 = [vertices[4*i3+t] for t in range(3)]

      if v1[1] != v2[1] or v1[1] != v3[1] or v2[1] != v3[1]: continue

      y = v1[1]

      if v1[0]==v2[0] and v1[2]==v3[2]: # |\
        
        dz = v2[2] - v1[2]
        dx = v3[0] - v1[0]

        if v2[2]+dz >= size2-STEP or v3[0]+dx >= size2-STEP: continue

        ii1 = i2
        ii2 = vdict[(str(v2[0]), str(v2[2]+dz))]
        ii3 = vdict[(str(v3[0]), str(v2[2]))]
        idx1 = (ii1, ii2, ii3)
        if vertices[4*ii2+1] != y or vertices[4*ii3+1] != y or idx1 not in triangles: continue

        up = ii2

        ii1 = i2
        ii2 = vdict[(str(v3[0]), str(v2[2]))]
        ii3 = i3
        idx2 = (ii1, ii2, ii3)
        if vertices[4*ii2+1] != y or idx2 not in triangles: continue

        ii1 = i3
        ii2 = vdict[(str(v3[0]), str(v2[2]))]
        ii3 = vdict[(str(v3[0]+dx), str(v3[2]))]
        idx3 = (ii1, ii2, ii3)
        if vertices[4*ii2+1] != y or vertices[4*ii3+1] != y or idx3 not in triangles: continue

        right = ii3

        print len(triangles)
        triangles.remove((i1,i2,i3))
        triangles.remove(idx1)
        triangles.remove(idx2)
        triangles.remove(idx3)
        triangles.add((i1, up, right))
        changed = True
        break

      else: # \|

        dz = v2[2] - v3[2]
        dx = v2[0] - v1[0]

        if v1[0]-dx <= -size2+STEP or v3[2]-dz <= -size2+STEP: continue

        ii1 = vdict[(str(v1[0]-dx), str(v1[2]))]
        ii2 = i1
        ii3 = vdict[(str(v1[0]), str(v3[2]))]
        idx1 = (ii1, ii2, ii3)
        if vertices[4*ii1+1] != y or vertices[4*ii3+1] != y or idx1 not in triangles: continue

        left = ii1

        ii1 = vdict[(str(v1[0]), str(v3[2]))]
        ii2 = i1
        ii3 = i3
        idx2 = (ii1, ii2, ii3)
        if vertices[4*ii1+1] != y or idx2 not in triangles: continue

        ii1 = vdict[(str(v1[0]), str(v3[2]))]
        ii2 = i3
        ii3 = vdict[(str(v3[0]), str(v3[2]-dz))]
        idx3 = (ii1, ii2, ii3)
        if vertices[4*ii1+1] != y or vertices[4*ii3+1] != y or idx3 not in triangles: continue

        bottom = ii3

        print len(triangles)
        triangles.remove((i1,i2,i3))
        triangles.remove(idx1)
        triangles.remove(idx2)
        triangles.remove(idx3)
        triangles.add((left, i2, bottom))
        changed = True
        break

        
  print "  Reduced to = ", len(triangles)

  #from operator import add
  #indices = reduce(lambda x,y: x+list(y), list(triangles), [])

  print "  Saving output..."

  out = file(output, "wb")

  out.write(pack(*(["4B"] + [ord(x) for x in ["H", "M", "A", "P"]])))

  out.write(pack(*(["ii"] + [len(vertices)/4, len(triangles)])))

  out.write(pack(*(["f"*(len(vertices)+len(normals)+len(uv))] + vertices + normals + uv)))
  
  for i in triangles:
    out.write(pack(*(["H"*3] + list(i))))
  
  out.close()

for root, dir, files in os.walk(DATA_PATH):
  for f in files:
    name = os.path.join(root, f)
    if name[-4:] == ".tga":
      print name
      process(name, name[:-4] + ".hmap")
