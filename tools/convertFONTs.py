import os
from xml.etree.ElementTree import ElementTree 
from struct import pack

DATA_PATH = os.path.join("..", "bin", "data")

def process(input, output):
  x = ElementTree(file=input)

  info = x._root[0]
  common = x._root[1]
  page = x._root[2]
  childs = x._root[3:]

  if childs[0].tag != "char":
    print "Unsupported font format!"
    return

  tmp = info.attrib["size"], common.attrib["scaleW"], common.attrib["scaleH"], common.attrib["lineHeight"], len(childs), int(childs[-1].attrib["id"])+1

  out = file(output, "wb")

  out.write(pack(*(["4B"] + [ord(x) for x in ["F", "O", "N", "T"]])))

  out.write(pack(*(["hhhhhh"] + [int(x) for x in tmp])))

  for c in childs:
    tmp = [c.attrib["id"], c.attrib["x"], c.attrib["y"], c.attrib["width"], c.attrib["height"], c.attrib["xoffset"], c.attrib["yoffset"], c.attrib["xadvance"]]
    out.write(pack(*["hhhbbbbb"] + [int(x) for x in tmp]))

  out.close()

for root, dir, files in os.walk(DATA_PATH):
  for f in files:
    name = os.path.join(root, f)
    if name[-4:] == ".fnt":
      print name
      #try:
      process(name, name[:-4] + ".font")
      #except:
      #  print "Exception!"
