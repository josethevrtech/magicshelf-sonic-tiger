#!/usr/bin/env python3
"""Install a validated private review package with the installed host transaction."""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,tempfile,time
root=Path.home()/'.local/share/magicshelf'
sys.path.insert(0,str((root/'current').resolve()))
from magicshelf.manager import Manager
from magicshelf.packages import unpack
from magicshelf.session import service_environment
from magicshelf.contracts import load
archive=Path(sys.argv[1]).resolve();content=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
key='sonic-tiger';env=service_environment()
backup=archive.parent/('install-backup-'+str(time.time_ns()));backup.mkdir()
for name in ['state.json','object-dock.json','sessions.json']:
 path=Path.home()/'.config/magicshelf'/name
 if path.exists():shutil.copy2(path,backup/name)
subprocess.run(['systemctl','--user','stop','magicshelf.service'],env=env,check=True)
try:
 manager=Manager();assert not manager.state['modules'].get(key,{}).get('enabled'),'Turn this creation Off before a private update'
 with tempfile.TemporaryDirectory(dir=root) as temp:
  stage=Path(temp)/'unpacked';digest=hashlib.sha256(archive.read_bytes()).hexdigest();meta=unpack(archive,stage,digest)
  assert meta['id']==key and meta['architecture']=='aarch64'
  data=root/'module-data'/key/'content'
  if content is not None and data.exists():
   for name in ['n71.program','n71.melody','tsonic.svg']:
    assert (data/name).read_bytes()==(content/name).read_bytes(),'Existing local content differs'
  elif content is not None:
   data.parent.mkdir(parents=True,exist_ok=True);shutil.copytree(content,data)
  with manager.transaction():manager.commit_package(key,meta['version'],stage,source=None)
 print(json.dumps({'installed':key,'version':meta['version'],'backup':str(backup),'private_content':str(data)}))
finally:
 subprocess.run(['systemctl','--user','start','magicshelf.service'],env=env,check=True)
