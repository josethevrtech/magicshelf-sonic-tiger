#!/usr/bin/env python3
"""Make a matching source release with pinned SDK/core source exports, no Git metadata."""
from pathlib import Path
import gzip,hashlib,io,json,subprocess,tarfile

r=Path(__file__).resolve().parents[1]
assert not subprocess.check_output(['git','-C',str(r),'status','--porcelain']).strip(),'Commit reviewed source before bundling'
revision=subprocess.check_output(['git','-C',str(r),'rev-parse','HEAD'],text=True).strip()
version=json.loads((r/'module.json').read_text())['version'];prefix='sonic-tiger-v'+version+'/'
out=r/'dist'/('publish-v'+version);out.mkdir(parents=True,exist_ok=True)
destination=out/('sonic-tiger-'+version+'-source.tar.gz')
git_tar=subprocess.check_output(['git','-C',str(r),'archive','--format=tar','--prefix='+prefix,'HEAD'])
payload=io.BytesIO()
with gzip.GzipFile(fileobj=payload,mode='wb',mtime=0,filename='') as gz:
 with tarfile.open(fileobj=gz,mode='w') as archive:
  def add(name,content,mode=0o644):
   info=tarfile.TarInfo(prefix+name);info.mode=mode;info.size=len(content);info.mtime=0;archive.addfile(info,io.BytesIO(content))
  with tarfile.open(fileobj=io.BytesIO(git_tar)) as source:
   for member in source.getmembers():
    if member.isfile():add(member.name.removeprefix(prefix),source.extractfile(member).read(),member.mode)
  for folder,lockfile in [('magicshelf','sdk.lock.json'),('lcd-emulator','emulator.lock.json')]:
   lock=json.loads((r/lockfile).read_text());base=r/'vendor'/folder
   actual=subprocess.check_output(['git','-C',str(base),'rev-parse','HEAD'],text=True).strip();assert actual==lock['revision']
   assert not subprocess.check_output(['git','-C',str(base),'status','--porcelain']).strip()
   names=subprocess.check_output(['git','-C',str(base),'ls-files','-z']).decode().split('\0');hashes={}
   for name in sorted(n for n in names if n):
    p=Path(name)
    if folder=='magicshelf' and not (name.startswith(('native/','schemas/','sdk/')) or (name.startswith('magicshelf/') and p.suffix=='.py') or name=='tools/package_creation.py' or name in {'LICENSE','OPENVR-LICENSE.txt','catalog.json','collections.json','shelf.json'}):continue
    assert not (base/name).is_symlink();content=(base/name).read_bytes();hashes[name]=hashlib.sha256(content).hexdigest()
    add('vendor/'+folder+'/'+name,content,0o755 if (base/name).stat().st_mode&0o111 else 0o644)
   export={'repository':lock['repository'],'revision':lock['revision'],'files':hashes}
   add('vendor/'+folder+'/SOURCE_EXPORT.json',(json.dumps(export,indent=2)+'\n').encode())
  add('SOURCE_REVISION.json',(json.dumps({'creation_revision':revision,'version':version},indent=2)+'\n').encode())
  add('SOURCE_BUILD.md',b'# Building this source release\n\nPinned SDK and native reference sources are included under vendor/ with verified export manifests. Run ./build.sh on Steam Frame or compatible ARM64 SteamOS with the development libraries documented in README. No ROM is required for the online creation. Private native-reference gameplay tests skip when local/content is absent.\n')
content=payload.getvalue()
if destination.exists():assert destination.read_bytes()==content,'Source archive already exists with different bytes'
else:destination.write_bytes(content)
print(destination,hashlib.sha256(content).hexdigest())
