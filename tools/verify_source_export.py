import hashlib,json
from pathlib import Path

def verify_export(target,lock):
    target=Path(target);marker=target/'SOURCE_EXPORT.json'
    if not marker.exists():return False
    data=json.loads(marker.read_text())
    if data['repository']!=lock['repository'] or data['revision']!=lock['revision']:
        raise SystemExit('Bundled source does not match dependency pin')
    for name,digest in data['files'].items():
        p=target/name
        if not p.resolve().is_relative_to(target.resolve()) or p.is_symlink() or hashlib.sha256(p.read_bytes()).hexdigest()!=digest:
            raise SystemExit('Bundled source differs from its export: '+name)
    return True
