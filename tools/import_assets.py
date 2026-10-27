from pathlib import Path
import json,struct,hashlib,zipfile,xml.etree.ElementTree as E,copy,io,re
from PIL import Image
import cairosvg
root=Path(__file__).resolve().parents[1]
def model(source):
 b=source.read_bytes();n=struct.unpack_from('<I',b,12)[0];d=json.loads(b[20:20+n]);binary=b[28+n:]
 def acc(i):
  a=d['accessors'][i];v=d['bufferViews'][a['bufferView']];fmt='<'+{5126:'f',5125:'I',5123:'H'}[a['componentType']]*{'VEC3':3,'VEC2':2,'SCALAR':1,'VEC4':4}[a['type']];s=v.get('byteOffset',0)+a.get('byteOffset',0);return [struct.unpack_from(fmt,binary,s+j*v.get('byteStride',struct.calcsize(fmt))) for j in range(a['count'])]
 p=acc(0);normal=acc(1);uv=acc(3);indices=acc(4);scale=.14/248.04281616210938;records=[];removed=0
 for t in range(0,len(indices),3):
  tri=[indices[t+i][0] for i in range(3)]
  if all(.79<uv[i][0]<.985 and .02<uv[i][1]<.281 and abs(p[i][1]-11.904144287109375)<.001 for i in tri):removed+=1;continue
  for i in tri:
   x,y,z=p[i];nx,ny,nz=normal[i];u,v=uv[i];records.append(((x-121.6)*scale,(z-103.458)*scale,(18.20575-y)*scale,nx,nz,-ny,u,v))
 out=root/'assets';out.mkdir(exist_ok=True)
 (out/'device.mesh').write_bytes(b'DPH2'+struct.pack('<II',1,len(records))+struct.pack('<IIff',0,len(records),.8,0)+b''.join(struct.pack('<8f',*r) for r in records))
 v=d['bufferViews'][d['images'][0]['bufferView']];im=Image.open(io.BytesIO(binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']])).convert('RGBA');im.thumbnail((2048,2048));im.putalpha(0);(out/'body-0.rgba').write_bytes(struct.pack('<II',*im.size)+im.tobytes())
 (out/'model-info.json').write_text(json.dumps(dict(source_sha256=hashlib.sha256(b).hexdigest(),credits=d['asset']['extras'],removed_screen_triangles=removed,triangles=len(records)//3,scale=scale),indent=2))
 print('Imported model:',len(records)//3,'triangles; removed baked screen:',removed)
def rom(source,destination):
 expected={'n71.program':'bf8d0ab88d153fabc688ffec19959209ca79c3db','n71.melody':'81cb75d73fab4479cd92fcb13d9cb03cec2afdd5','tsonic.svg':'fa944958eb64e8283d35951ff105cead28e6ab8a'}
 with zipfile.ZipFile(source) as z:
  blobs={n:z.read(n) for n in expected}
 for n,h in expected.items():
  if hashlib.sha1(blobs[n]).hexdigest()!=h:raise ValueError('ROM mismatch: '+n)
 destination.mkdir(parents=True,exist_ok=True)
 for n,b in blobs.items():(destination/n).write_bytes(b)
 ns='{http://www.w3.org/2000/svg}';svg=E.fromstring(blobs['tsonic.svg']);groups=[]
 for node in svg.iter():
  title=node.find(ns+'title')
  if title is not None and re.fullmatch(r'[0-3]\.[0-9]+\.[0-3]',title.text or ''):groups.append((node,title.text))
 target=destination/'segments';target.mkdir(exist_ok=True);meta=[]
 parents={c:p for p in svg.iter() for c in p}
 for node,key in groups:
  doc=E.Element(ns+'svg',{'viewBox':svg.attrib['viewBox'],'width':'758','height':'540'})
  chain=[];p=parents.get(node)
  while p is not None and p is not svg:chain.append(p);p=parents.get(p)
  cursor=doc
  for ancestor in reversed(chain):cursor=E.SubElement(cursor,ns+'g',{k:v for k,v in ancestor.attrib.items() if k in ('transform','style')})
  cursor.append(copy.deepcopy(node));im=Image.open(io.BytesIO(cairosvg.svg2png(bytestring=E.tostring(doc),output_width=758,output_height=540))).convert('RGBA');box=im.getbbox()
  if box:im.crop(box).save(target/(key+'.png'));meta.append(dict(key=key,box=box))
 (destination/'segments.json').write_text(json.dumps(meta))
 print('Verified Sonic ROM and rasterized',len(meta),'LCD segments')
if __name__=='__main__':
 import argparse
 p=argparse.ArgumentParser();p.add_argument('--model',type=Path);p.add_argument('--rom',type=Path);p.add_argument('--destination',type=Path,default=root/'local/content');a=p.parse_args()
 if a.model:model(a.model)
 if a.rom:rom(a.rom,a.destination)
