from pathlib import Path
from PIL import Image,PngImagePlugin
import hashlib,json
r=Path(__file__).resolve().parents[1]
source='https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c'
proof=[]
for p in sorted((r/'media').glob('sonic-tiger-*.png')):
 im=Image.open(p);before=im.convert('RGBA').tobytes();meta=PngImagePlugin.PngInfo()
 fields={'Title':p.stem.replace('-',' ').title(),'Software':'MagicShelf Sonic Tiger LCD native renderer',
 'Attribution':f'Tiger Handheld Games Console by Mattyew, {source}, CC BY 4.0. Modified: mesh conversion, scale, textures and live screen.',
 'License':'Model: https://creativecommons.org/licenses/by/4.0/ . Game, characters and artwork retain their original rights.',
 'Game source':'https://archive.org/details/hh_tsonic'}
 for k,v in fields.items():meta.add_text(k,v)
 im.save(p,pnginfo=meta,optimize=True)
 assert Image.open(p).convert('RGBA').tobytes()==before
 proof.append({'file':p.name,'width':im.width,'height':im.height,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(r/'media/screenshots.json').write_text(json.dumps(proof,indent=2)+'\n')
(r/'media/README.md').write_text('''# Screenshots and attribution

These captures use the real MagicShelf renderer and supplied Mattyew mesh. Gameplay pixels came from the actual Archive.org MAME player captured on Steam Frame during an isolated test. The recorded complete player frame was rendered through the same native screen-texture path. Settings are the actual creation settings UI in an isolated temporary profile. The back view shows the supplied geometry as-is. A neutral background was added; game scores and artwork were not invented. No AI-generated imagery is used.

Model: [Tiger Handheld Games Console]('''+source+''') by [Mattyew](https://sketchfab.com/mattyu), [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). The original download credits the artist at https://sketchfab.com/mrgrandgrind; that supplied attribution is retained in ASSET_CREDITS.md. Modifications: mesh conversion, scale, material texture conversion and live screen replacement.

Game/player: [Internet Archive Sonic handheld](https://archive.org/details/hh_tsonic). Sega/Tiger game material retains its original rights. The model's CC license does not relicense the game imagery. Each PNG also contains attribution/source metadata. See screenshots.json for dimensions and checksums.
''')
print(json.dumps(proof,indent=2))
