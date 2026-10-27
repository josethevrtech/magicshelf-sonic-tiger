from pathlib import Path
import json
r=Path(__file__).resolve().parents[1];p=r/'module.json';m=json.loads(p.read_text());version='0.2.3';repo='https://github.com/josethevrtech/magicshelf-sonic-tiger';source='https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c'
m.update(version=version,price=0,category='Games',tagline='Pick it up. Play Sonic.',minimum_framework_version='0.3.21',startup_timeout_sec=30,
 description='Play Sonic the Hedgehog from Internet Archive inside a 3D Tiger handheld on Steam Frame. Internet required. No ROM import.',
 about='''SONIC ON YOUR SHELF
Play the original Sonic the Hedgehog LCD game through Internet Archive's online player. Pick up the handheld, place it nearby, or return it to your MagicShelf.

START PLAYING
An internet connection is needed to load the player. Wait for the game screen, then press X or point at ON/START and pull the trigger. No ROM import is needed.

CONTROLS
Directions: move and jump. A: Spin. B: Pause. X: ON/START. Y: Settings. Point at the model buttons with your free hand and hold the trigger to hold a key. The two ATTACK halves activate Spin; ACTION jumps. ACL resets the game.

PICK UP AND PLACE
Hold grip to pick up; release to place. The holding stick adjusts distance. Home or a deliberate shake returns it to the shelf.

SOUND AND CONNECTION
Y opens sound, volume and Reload Online Player. The handheld plays its own sound cues, not a continuous soundtrack. If loading fails, use Retry. Archive.org availability and loading times can vary.

PLEASE KNOW
The model is Mattyew's Sonic 2 casing; the online game is Sonic 1. Settings are saved. The online game restarts when turned Off or reloaded. This is a free fan-made integration, not an official Sega, Tiger Electronics or Internet Archive product.''')
m['screenshots']=[{'url':f'{repo}/releases/download/v{version}/{name}','caption':caption} for name,caption in [
 ('sonic-tiger-gameplay.png','Live Sonic 1 gameplay and original screen background inside Mattyew\'s Sonic 2 handheld model.'),
 ('sonic-tiger-settings.png','The creation\'s sound, volume and online-player settings.'),
 ('sonic-tiger-back.png','Back and side of Mattyew\'s handheld model.')]]
m['links']=[{'label':label,'url':url} for label,url in [
 ('Sonic Tiger LCD source',repo),('Report an issue',repo+'/issues'),('Play the original Archive.org item','https://archive.org/details/hh_tsonic'),
 ('Mattyew\'s handheld model',source),('Model license: CC BY 4.0','https://creativecommons.org/licenses/by/4.0/'),
 ('Emularity source','https://github.com/db48x/emularity'),('MAME source','https://github.com/mamedev/mame'),('MagicShelf framework','https://github.com/josethevrtech/magicshelf')]]
m['credits']=[
 {'name':'Mattyew','role':'Tiger Handheld Games Console 3D model; converted, scaled and fitted with a live screen.','url':source,'license':'CC BY 4.0. https://creativecommons.org/licenses/by/4.0/ Artist: https://sketchfab.com/mattyu'},
 {'name':'Internet Archive','role':'Hosts the online Sonic handheld player and its original screen artwork.','url':'https://archive.org/details/hh_tsonic','license':'Hosted content retains its original rights; no game files are bundled.'},
 {'name':'Daniel Brooks and Emularity contributors','role':'Browser emulator loader used by Internet Archive.','url':'https://github.com/db48x/emularity','license':'GPL-3.0; hosted by Archive.org, not bundled.'},
 {'name':'MAMEDev and contributors','role':'MAME emulation used by the hosted player.','url':'https://www.mamedev.org/','license':'GPL-2.0-or-later; individual source files may use other licenses.'},
 {'name':'Sega / Tiger Electronics','role':'Original Sonic the Hedgehog characters and handheld game.','url':'https://archive.org/details/hh_tsonic','license':'Original game, characters, artwork and trademarks retain their respective rights; not relicensed.'},
 {'name':'Qt Project and Chromium contributors','role':'System browser runtime and application libraries.','url':'https://doc.qt.io/qt-6.10/qtwebengine-licensing.html','license':'Qt open-source and Chromium third-party licenses. System libraries are dynamically linked.'},
 {'name':'Valve','role':'OpenVR integration.','url':'https://github.com/ValveSoftware/openvr','license':'BSD-3-Clause; see OPENVR-LICENSE.txt.'},
 {'name':'bzhxx / hap','role':'Retained native SM511 development reference; not the online backend.','url':'https://github.com/bzhxx/LCD-Game-Emulator','license':'GPL-3.0-or-later system code; BSD-3-Clause CPU source. See THIRD_PARTY_NOTICES.md.'}
]
p.write_text(json.dumps(m,indent=2)+'\n');print('About characters:',len(m['about']),'credits:',len(m['credits']))
