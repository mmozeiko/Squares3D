import os

SRC_PATH = os.path.join("..", "src")

def process(infile, outfile):
  f = file(infile, "r")
  data = f.readlines()
  f.close()

  newdata = [x.rstrip() for x in data]
  if newdata[-1] != "": newdata.append("")
            
  f = file(outfile, "w")
  f.write("\n".join(newdata))
  f.close()

for root, dir, files in os.walk(SRC_PATH):
  for f in files:
    name = os.path.join(root, f)
    if name[-4:] == ".cpp" or name[-2:] == ".h":
      print name
      process(name, name)
