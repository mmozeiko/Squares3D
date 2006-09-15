import os

DATA_PATH = os.path.join("..", "bin", "data")

def process(infile, outfile):
  f = open(infile, "rb")
  data = f.read()
  f.close()
  if ord(data[2]) == 10: return
  if ord(data[2]) != 2: raise "Unsupported TGA format!"
  for offset in [1,3,4,5,6,7,8,9,10,11]:
    if ord(data[offset]) != 0:
      raise "Unsupported TGA format!"
  
  X = ord(data[12]) + ord(data[13])*256
  Y = ord(data[14]) + ord(data[15])*256
  bpp = ord(data[16])
  if (bpp != 24 and bpp != 32): raise "Unsupported TGA format!"
  bpp /= 8
  
  newdata = [0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, ord(data[12]), ord(data[13]), ord(data[14]), ord(data[15]), ord(data[16]), ord(data[17])]
  idx = 18 + ord(data[0])
  
  first = True
  while idx < len(data):
    pix = [ord(data[idx]), ord(data[idx+1]), ord(data[idx+2])]
    if bpp==4: pix.append(ord(data[idx+3]))
    idx += bpp
    
    if first:
      last = pix[:]
      unique = []
      count = 1
      first = False
    else:
      if last==pix:
        count += 1
      if last != pix or count==128:
        if count==1:
          unique.append(last)
        else:
          while len(unique) != 0:
            size = len(unique)
            if size>128: size = 128
            newdata.append(0x00 + size-1)
            for i in range(size):
              newdata += unique[i]
            unique = unique[size:]
            
          newdata.append(0x80 + count-1)
          newdata += last
          if count==128:
            first = True
          else:
            count = 1

        last = pix

  if count==1: unique.append(last)
  
  while len(unique) != 0:
      size = len(unique)
      if size>128: size = 128
      newdata.append(0x00 + size-1)
      for i in range(size):
        newdata += unique[i]
      unique = unique[size:]
        
  if count>1:
    newdata.append(0x80 + count-1)
    newdata += last
            
  if len(newdata)<len(data):
    newdata = [chr(c) for c in newdata]
    f = open(outfile, "wb")
    for c in newdata:
      f.write(c)
    f.close()

for root, dir, files in os.walk(DATA_PATH):
  for file in files:
    name = os.path.join(root, file)
    if name[-4:] == ".tga":
      print name
      try:
        process(name, name)
      except:
        pass
        #print "Exception:", e
