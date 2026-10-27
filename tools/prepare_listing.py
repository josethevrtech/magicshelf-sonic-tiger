from pathlib import Path
import json
r=Path(__file__).resolve().parents[1];p=r/'module.json';m=json.loads(p.read_text());version='0.3.1';repo='https://github.com/josethevrtech/magicshelf-sonic-tiger';source='https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c'
m.update(version=version,price=0,category='Games',tagline='Pick it up. Play Sonic.',minimum_framework_version='0.3.21',startup_timeout_sec=30,
 description='Play Sonic the Hedgehog 2 from PEXY inside a 3D Tiger handheld on Steam Frame. Internet required. No ROM import.',
 about='''SONIC ON YOUR SHELF
Play Sonic the Hedgehog 2 through PEXY's online player. Pick up the handheld, place it nearby, or return it to your MagicShelf.

START PLAYING
An internet connection is needed to load the player. Wait for the game screen, then press X or point at ON/START and pull the trigger. No ROM import is needed.

CONTROLS
Directions: move and jump. A: Action. B: Pause. X: ON/START. Y: Settings. Point at the model buttons with your free hand and hold the trigger to hold a key. The two ATTACK halves and ACTION activate the same Action input. ACL resets the game.

PICK UP AND PLACE
The handheld fits its shelf slot automatically. Hold grip to pick up at your chosen size; release to place. The holding stick adjusts distance. Home or a deliberate shake returns it to the shelf.

SOUND AND CONNECTION
Y opens sound, volume and Reload Online Player. The handheld plays its own sound cues, not a continuous soundtrack. If loading fails, use Retry. PEXY availability and loading times can vary.

PLEASE KNOW
The model and online game are Sonic 2. Settings are saved. Progress saves locally on Off and reload; compatibility depends on the hosted player version. This is a free fan-made integration, not an official Sega, Tiger Electronics or PEXY product.''')
m['screenshots']=[{'url':f'{repo}/releases/download/v{version}/{name}','caption':caption} for name,caption in [
 ('sonic-tiger-gameplay.png','Live Sonic 2 gameplay and original screen background inside Mattyew\'s Sonic 2 handheld model.'),
 ('sonic-tiger-settings.png','The creation\'s sound, volume and online-player settings.'),
 ('sonic-tiger-back.png','Back and side of Mattyew\'s handheld model.')]]
m['links']=[{'label':label,'url':url} for label,url in [
 ('Sonic Tiger LCD source',repo),('Report an issue',repo+'/issues'),('Play Sonic 2 on PEXY','https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/'),
 ('Mattyew\'s handheld model',source),('Model license: CC BY 4.0','https://creativecommons.org/licenses/by/4.0/'),
 ('MAME source','https://github.com/mamedev/mame'),('MagicShelf framework','https://github.com/josethevrtech/magicshelf')]]
m['credits']=[
 {'name':'Mattyew','role':'Tiger Handheld Games Console 3D model; converted, scaled and fitted with a live screen.','url':source,'license':'CC BY 4.0. https://creativecommons.org/licenses/by/4.0/ Artist: https://sketchfab.com/mattyu'},
 {'name':'PEXY','role':'Hosts the online Sonic 2 handheld player and its original screen artwork.','url':'https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/','license':'Hosted content retains its original rights; no game files are bundled.'},
 {'name':'MAMEDev and contributors','role':'MAME emulation used by the hosted player.','url':'https://www.mamedev.org/','license':'GPL-2.0-or-later; individual source files may use other licenses.'},
 {'name':'Sega / Tiger Electronics','role':'Original Sonic the Hedgehog characters and handheld game.','url':'https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/','license':'Original game, characters, artwork and trademarks retain their respective rights; not relicensed.'},
 {'name':'Qt Project and Chromium contributors','role':'System browser runtime and application libraries.','url':'https://doc.qt.io/qt-6.10/qtwebengine-licensing.html','license':'Qt open-source and Chromium third-party licenses. System libraries are dynamically linked.'},
 {'name':'Valve','role':'OpenVR integration.','url':'https://github.com/ValveSoftware/openvr','license':'BSD-3-Clause; see OPENVR-LICENSE.txt.'},
 {'name':'bzhxx / hap','role':'Retained native SM511 development reference; not the online backend.','url':'https://github.com/bzhxx/LCD-Game-Emulator','license':'GPL-3.0-or-later system code; BSD-3-Clause CPU source. See THIRD_PARTY_NOTICES.md.'}
]
p.write_text(json.dumps(m,indent=2)+'\n');print('About characters:',len(m['about']),'credits:',len(m['credits']))
