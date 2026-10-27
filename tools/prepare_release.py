#!/usr/bin/env python3
"""Validate the candidate and prepare publication files, without publishing."""
from pathlib import Path
import difflib,hashlib,json,shutil,subprocess,sys,zipfile
from PIL import Image

r=Path(__file__).resolve().parents[1]
sdk=r/'vendor/magicshelf';sys.path.insert(0,str(sdk))
from magicshelf.contracts import manifest,catalog
from magicshelf.storefront import valid_png

meta=manifest(json.loads((r/'module.json').read_text()))
version=meta['version'];release=r/'release';release.mkdir(exist_ok=True)
out=r/'dist'/('publish-v'+version);out.mkdir(parents=True,exist_ok=True)
package_root=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else r/'build/package'
subprocess.run([sys.executable,str(sdk/'tools/package_creation.py'),str(package_root),
 '--repository','josethevrtech/magicshelf-sonic-tiger','--output',str(out)],check=True,stdout=subprocess.DEVNULL)
candidate=catalog(json.loads((out/'catalog.json').read_text()))['modules'][0]
assert candidate=={**meta,'url':candidate['url'],'sha256':candidate['sha256'],'size_bytes':candidate['size_bytes']}

screens=[]
for picture in meta['screenshots']:
 name=picture['url'].rsplit('/',1)[1];source=r/'media'/name;valid_png(source)
 image=Image.open(source)
 assert 'Mattyew' in image.text.get('Attribution','')
 assert 'creativecommons.org/licenses/by/4.0' in image.text.get('License','')
 shutil.copy2(source,out/name)
 screens.append({'file':name,'size':list(image.size),'sha256':hashlib.sha256(source.read_bytes()).hexdigest()})

archive=out/(meta['id']+'-'+version+'-linux-aarch64.magicshelf')
forbidden_hashes={'bf8d0ab88d153fabc688ffec19959209ca79c3db','81cb75d73fab4479cd92fcb13d9cb03cec2afdd5','fa944958eb64e8283d35951ff105cead28e6ab8a'}
with zipfile.ZipFile(archive) as package:
 names=package.namelist()
 assert {'bin/creation','bin/archive-player','MODEL-LICENSE.txt','ASSET_CREDITS.md','THIRD_PARTY_NOTICES.md'}.issubset(names)
 for name in names:
  assert not any(part in {'local','module-data','runtime-data','.git'} for part in Path(name).parts)
  assert not name.endswith(('.program','.melody','.svg','.zip','.glb'))
  assert hashlib.sha1(package.read(name)).hexdigest() not in forbidden_hashes
 assert json.loads(package.read('module.json'))==meta
 for picture in screens:assert hashlib.sha256(package.read('media/'+picture['file'])).hexdigest()==picture['sha256']

collection='https://github.com/josethevrtech/magicshelf-sonic-tiger/releases/latest/download/catalog.json'
entry={**candidate,'collection_url':collection}
(release/'catalog-entry.json').write_text(json.dumps(entry,indent=2)+'\n')
base=release/'framework-base';patch=[]
for name in ['catalog.json','collections.json']:
 before=(base/name).read_text();data=json.loads(before)
 if name=='catalog.json':
  data['modules']=[entry if m['id']==meta['id'] else m for m in data['modules']]
  if not any(m['id']==meta['id'] for m in data['modules']):data['modules'].append(entry)
  catalog(data)
 else:
  if collection not in data['catalogs']:data['catalogs'].append(collection)
 after=json.dumps(data,indent=2)+'\n'
 patch.extend(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
(release/'magicshelf-catalog.patch').write_text(''.join(patch))

lines=['# '+meta['name'],'',meta['tagline'],'',meta['description'],'','## About','',meta['about'],'','## Credits','']
for credit in meta['credits']:lines.append('- ['+credit['name']+']('+credit['url']+'): '+credit['role']+' '+credit.get('license',''))
lines+=['','## Screenshots','']
for picture in meta['screenshots']:lines+=['!['+picture['caption']+'](../media/'+picture['url'].rsplit('/',1)[1]+')','']
(release/'LISTING.md').write_text('\n'.join(lines))

for name in ['ASSET_CREDITS.md','THIRD_PARTY_NOTICES.md','LICENSE','SDK-LICENSE.txt','OPENVR-LICENSE.txt']:
 shutil.copy2(r/name,out/name)
shutil.copy2(r/'assets/model-license.txt',out/'MODEL-LICENSE.txt')
if (release/'RELEASE_NOTES.md').exists():shutil.copy2(release/'RELEASE_NOTES.md',out/'RELEASE_NOTES.md')
checks={'version':version,'architecture':meta['architecture'],'package':archive.name,'package_sha256':candidate['sha256'],
 'package_bytes':archive.stat().st_size,'package_files':len(names),'screenshots':screens,
 'manifest_and_catalog_valid':True,'no_rom_or_private_content':True,'packaged_images_match_release_assets':True,
 'framework_patch_base':json.loads((base/'provenance.json').read_text()),
 'publication_state':'verified release artifacts; source and release are published independently of the framework catalog',
 'physical_validation':'User confirmed 3D ON/START, visible handheld and native audio. Remaining individual controls and shake/snap need full headset confirmation.'}
(release/'CHECKS.json').write_text(json.dumps(checks,indent=2)+'\n')
print(json.dumps(checks,indent=2))
