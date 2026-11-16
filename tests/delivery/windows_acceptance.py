"""Install, exercise, and uninstall the actual NSIS package on an isolated runner."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time
import winreg

from terminal_playthrough import play, map_fingerprints, Terminal

root = Path(__file__).resolve().parents[2]
evidence = root / 'dist/evidence'
evidence.mkdir(parents=True, exist_ok=True)
install = Path(os.environ['LOCALAPPDATA']) / 'Programs/CircuitoEscape'
generated = Path(os.environ['LOCALAPPDATA']) / 'CircuitoEscape/generated-maps'
shortcuts = Path(os.environ['APPDATA']) / 'Microsoft/Windows/Start Menu/Programs/Circuito de Escape'
desktop = Path(os.environ['USERPROFILE']) / 'Desktop/Circuito de Escape.lnk'
registry = r'Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape'
assert not install.exists() and not generated.exists(), 'Use a fresh runner/account'
report = []


def passed(message):
    report.append(message)
    (evidence / 'acceptance.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print('PASS ' + message, flush=True)


def run(argv, **kwargs):
    result = subprocess.run([str(a) for a in argv], text=True, capture_output=True, timeout=180, **kwargs)
    assert result.returncode == 0, f'{argv}: {result.returncode}\n{result.stdout}\n{result.stderr}'
    return result.stdout


run([root / 'dist/CircuitoEscape-Setup.exe', '/S'])
for relative in ['bin/escape_circuit.exe', 'bin/map_generator.exe', 'Uninstall.exe',
                 'assets/maps/mapa_facil.txt', 'assets/maps/mapa_medio.txt', 'assets/maps/mapa_dificil.txt']:
    assert (install / relative).is_file(), relative
for name in ['Jugar', 'Generar mapas', 'Jugar mapas generados', 'Desinstalar']:
    assert (shortcuts / (name + '.lnk')).is_file(), name
assert desktop.is_file()
with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY) as key:
    assert winreg.QueryValueEx(key, 'InstallLocation')[0] == str(install)
manifest = {}
for binary in sorted((install / 'bin').iterdir()):
    data = binary.read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[pe:pe + 4] == b'PE\0\0'
    assert struct.unpack_from('<H', data, pe + 4)[0] == 0x8664, binary
    manifest[binary.name] = hashlib.sha256(data).hexdigest()
assert any(name.endswith('.dll') for name in manifest), 'Runtime DLLs missing'
(evidence / 'installed-manifest.json').write_text(json.dumps(manifest, indent=2))
passed('NSIS silent installation, x64 PE executables/DLLs, maps, shortcuts, HKCU uninstall entry')

# A runner has development tools installed; remove them from runtime discovery.
os.environ['PATH'] = os.pathsep.join([str(install / 'bin'), os.environ['WINDIR'] + r'\System32', os.environ['WINDIR']])
for name in ['escape_circuit', 'map_generator']:
    run([install / f'bin/{name}.exe', '--help'])
passed('Installed binaries start with PATH restricted to package + Windows system directories')
play(install / 'bin/escape_circuit.exe', install / 'assets/maps', evidence, 'bundled')
passed('Bundled Easy/Medium/Hard: actual ConPTY H/Z and complete victory in all three; hashes/mtime unchanged')

# Exercise the exact generator launcher used by the Start menu with chosen dimensions.
terminal = Terminal([os.environ['COMSPEC'], '/d', '/c', str(install / 'launch-generator.cmd')])
try:
    terminal.contains('Rows (5..40):')
    terminal.send('15\r')
    terminal.contains('Columns (5..60')
    terminal.send('20\r')
    terminal.contains('Generated maps directory:', timeout=90)
    terminal.send('\r')  # launcher pause
    deadline = time.monotonic() + 10
    while terminal.process.isalive() and time.monotonic() < deadline:
        terminal.pump()
        time.sleep(0.05)
    assert not terminal.process.isalive()
    assert terminal.process.exitstatus == 0
finally:
    (evidence / 'generator-launcher.ansi.txt').write_text(''.join(terminal.raw), encoding='utf-8')
    if terminal.process.isalive():
        terminal.process.close(force=True)
for name in ['mapa_facil.txt', 'mapa_medio.txt', 'mapa_dificil.txt']:
    assert 'dimensions 15 20\n' in (generated / name).read_text()
passed('Start-menu generator launcher accepts terminal input 15 x 20 and writes three maps')
play(install / 'bin/escape_circuit.exe', generated, evidence, 'generated-15x20')
passed('Generated Easy/Medium/Hard: actual ConPTY H/Z and complete victory; offline files unchanged')

# Compare actual generator processes, not merely internal batch structures.
for workers in [1, 4]:
    target = evidence / f'workers-{workers}'
    log = run([install / 'bin/map_generator.exe', '--rows', '20', '--columns', '30',
        '--seed', '12345', '--workers', workers, '--initial-energy', '1000', '--maximum-energy', '1000',
        '--turn-limit', '2000', '--extra-openings', '15', '--search-states', '100', '--output', target])
    (evidence / f'workers-{workers}.txt').write_text(log)
for name in ['mapa_facil.txt', 'mapa_medio.txt', 'mapa_dificil.txt']:
    assert (evidence / 'workers-1' / name).read_bytes() == (evidence / 'workers-4' / name).read_bytes()
passed('Two installed generator processes workers=1/4 produce byte-identical maps')
preserved = map_fingerprints(generated)
# Normal NSIS self-copy/self-removal path, exactly as the user shortcut invokes it.
run([install / 'Uninstall.exe', '/S'])
for _ in range(200):
    if not install.exists():
        break
    time.sleep(0.1)
assert not install.exists(), 'Installation directory remains after uninstall'
assert not (install / 'bin').exists()
assert not (install / 'assets').exists()
assert not shortcuts.exists() and not desktop.exists()
try:
    winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
except FileNotFoundError:
    pass
else:
    raise AssertionError('Uninstall registry entry remains')
assert map_fingerprints(generated) == preserved
passed('Silent uninstall removes binaries/maps/shortcuts/registry and preserves personal generated maps')
