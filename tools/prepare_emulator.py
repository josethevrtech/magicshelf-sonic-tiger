#!/usr/bin/env python3
import json,pathlib,subprocess
r=pathlib.Path(__file__).resolve().parents[1]
lock=json.loads((r/'emulator.lock.json').read_text())
target=r/'vendor/lcd-emulator'
if not target.exists():
    subprocess.run(['git','clone','--no-checkout',lock['repository'],str(target)],check=True)
    subprocess.run(['git','-C',str(target),'checkout','--detach',lock['revision']],check=True)
actual=subprocess.check_output(['git','-C',str(target),'rev-parse','HEAD'],text=True).strip()
if actual!=lock['revision'] or subprocess.check_output(['git','-C',str(target),'status','--porcelain'],text=True).strip():
    raise SystemExit('Emulator checkout differs from pinned source')
