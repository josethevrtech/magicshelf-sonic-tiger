#!/usr/bin/env python3
"""Finish and verify the curated local upload folder. Never contacts GitHub."""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,tarfile,tempfile
r=Path(__file__).resolve().parents[1];meta=json.loads((r/'module.json').read_text());v=meta['version'];out=r/'dist'/('publish-v'+v)
sys.path.insert(0,str(r/'vendor/magicshelf'))
from magicshelf.contracts import catalog,manifest
from magicshelf.collections import catalogs

with tempfile.TemporaryDirectory(prefix='sonic-catalog-check-') as temp:
 t=Path(temp)
 for name in ['catalog.json','collections.json']:shutil.copy2(r/'release/framework-base'/name,t/name)
 patch=str(r/'release/magicshelf-catalog.patch')
 subprocess.run(['git','apply','--check',patch],cwd=t,check=True)
 subprocess.run(['git','apply',patch],cwd=t,check=True)
 proposed=catalog(json.loads((t/'catalog.json').read_text()))
 assert any(m['id']==meta['id'] for m in proposed['modules'])
 assert any('magicshelf-sonic-tiger' in url for url in catalogs(json.loads((t/'collections.json').read_text())))

subprocess.run([sys.executable,str(r/'tools/bundle_source.py')],check=True)
source=out/('sonic-tiger-'+v+'-source.tar.gz')
with tempfile.TemporaryDirectory(prefix='sonic-source-check-') as temp:
 with tarfile.open(source) as archive:
  for member in archive.getmembers():
   assert not any(p in {'local','.git','module-data','runtime-data'} for p in Path(member.name).parts)
   assert not member.name.endswith(('.program','.melody','.svg','.glb','.zip'))
  archive.extractall(temp,filter='data')
 exported=Path(temp)/('sonic-tiger-v'+v)
 subprocess.run([sys.executable,'tools/fetch_sdk.py'],cwd=exported,check=True)
 subprocess.run([sys.executable,'tools/prepare_emulator.py'],cwd=exported,check=True)
 env={**os.environ,'SONIC_NO_AUDIO':'1'}
 subprocess.run(['bash','./build.sh','--tests-only'],cwd=exported,env=env,check=True)

revision=subprocess.check_output(['git','-C',str(r),'rev-parse','HEAD'],text=True).strip()
for name in ['CHECKS.json','RUNTIME.json']:shutil.copy2(r/'release'/name,out/name)
record={'creation_revision':revision,'version':v,'framework_catalog_patch_applies':True,
 'source_exports_verified':True,'extracted_source_tests_pass':True,'source_has_no_rom_or_private_data':True,
 'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'published':False}
(out/'RELEASE_BUILD.json').write_text(json.dumps(record,indent=2)+'\n')
lines=[]
for path in sorted(out.iterdir()):
 if path.is_file() and path.name!='RELEASE_SHA256SUMS':lines.append(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+path.name)
(out/'RELEASE_SHA256SUMS').write_text('\n'.join(lines)+'\n')
print(json.dumps(record,indent=2));print('Curated release folder:',out)
