"""Exercise the shipping FTXUI binary with actual terminal input, never a replay hook.
Windows uses ConPTY; Linux uses pexpect. pyte reconstructs incremental ANSI frames.
"""
import argparse
import hashlib
import os
from pathlib import Path
import queue
import re
import threading
import time

import pyte


class Terminal:
    def __init__(self, argv):
        if os.name == 'nt':
            from winpty import PtyProcess
            self.process = PtyProcess.spawn(argv, dimensions=(60, 160), backend="0")
        else:
            import pexpect
            self.process = pexpect.spawn(argv[0], argv[1:], dimensions=(60, 160),
                                         encoding='utf-8', timeout=60)
        self.screen = pyte.Screen(160, 60)
        self.stream = pyte.Stream(self.screen)
        self.output = queue.Queue()
        self.raw = []
        threading.Thread(target=self._read, daemon=True).start()

    def _read(self):
        try:
            while True:
                data = self.process.read(4096) if os.name == 'nt' else self.process.read_nonblocking(4096, 60)
                if not data:
                    break
                self.output.put(data)
        except (EOFError, OSError):
            pass
        except Exception as exc:
            self.output.put(str(exc))

    def pump(self):
        while True:
            try:
                data = self.output.get_nowait()
            except queue.Empty:
                break
            self.raw.append(data)
            self.stream.feed(data)
        return '\n'.join(self.screen.display)

    def send(self, text):
        if os.name == 'nt':
            self.process.write(text)
        else:
            self.process.send(text)

    def until(self, predicate, description, timeout=35):
        deadline = time.monotonic() + timeout
        matched_since = None
        while time.monotonic() < deadline:
            frame = self.pump()
            if predicate(frame):
                if matched_since is None:
                    matched_since = time.monotonic()
                # ConPTY emits a single redraw in several chunks. Do not inspect
                # a half-painted frame as soon as the first HUD row arrives.
                if time.monotonic() - matched_since >= 0.2:
                    return frame
            else:
                matched_since = None
            time.sleep(0.03)
        raise AssertionError(f'Terminal timeout: {description}\n{self.pump()}')

    def contains(self, text, timeout=35):
        return self.until(lambda frame: text in frame, text, timeout)

    def finish(self):
        self.send('q')
        deadline = time.monotonic() + 10
        while self.process.isalive() and time.monotonic() < deadline:
            self.pump()
            time.sleep(0.05)
        assert not self.process.isalive(), 'Game did not exit after Q'
        if os.name != 'nt':
            self.process.close()
        assert self.process.exitstatus == 0, f'Game exit: {self.process.exitstatus}'


def map_fingerprints(directory):
    return {p.name: (hashlib.sha256(p.read_bytes()).hexdigest(), p.stat().st_mtime_ns)
            for p in sorted(Path(directory).glob('mapa_*.txt'))}


def play(executable, maps, evidence, label):
    maps, evidence = Path(maps).resolve(), Path(evidence)
    evidence.mkdir(parents=True, exist_ok=True)
    original = map_fingerprints(maps)
    assert len(original) == 3
    for level, filename in [('easy', 'mapa_facil.txt'), ('medium', 'mapa_medio.txt'), ('hard', 'mapa_dificil.txt')]:
        data = (maps / filename).read_text()
        actions = re.search(r'^actions (\d+) (.+)$', data, re.M)
        keys = {'U': 'w', 'D': 's', 'L': 'a', 'R': 'd', 'W': 'e'}
        route = actions[2].split()
        assert len(route) == int(actions[1])
        game = Terminal([str(Path(executable).resolve()), '--maps', str(maps), '--difficulty', level])
        prefix = evidence / f'{label}-{level}'
        try:
            game.contains('WELCOME')
            game.send('\r')
            initial = game.until(lambda f: all(t in f for t in ['Turn 0/', level.upper(), 'FULL MAP']),
                                 'complete initial gameplay frame')
            # HUD must remain identical through H, route selection and Z.
            hud = re.findall(r'Turn \d+/\d+|Energy \d+/\d+|Player \(\d+,\d+\)|Guard \(\d+,\d+\)|Switch A \w+ \| B \w+', initial)
            assert len(hud) == 5, hud
            game.send('h')
            game.contains('DIJKSTRA ENERGY HELP')
            game.send('z')
            frame = game.contains('DETAIL')
            for field in hud:
                assert field in frame, field
            game.send('1')
            game.send('2')
            game.send('3')
            game.send('0')
            game.send('h')
            game.until(lambda f: 'DIJKSTRA ENERGY HELP' not in f, 'close help')
            game.send('z')
            frame = game.contains('FULL MAP')
            for field in hud:
                assert field in frame, field
            prefix.with_suffix('.initial.txt').write_text(frame, encoding='utf-8')
            for turn, action in enumerate(route, 1):
                game.send(keys[action])
                if turn < len(route):
                    game.contains(f'Turn {turn}/', timeout=10)
            victory = game.contains('Circuit complete. You escaped.', timeout=35)
            assert 'VICTORY' in victory
            prefix.with_suffix('.victory.txt').write_text(victory, encoding='utf-8')
            game.finish()
            assert map_fingerprints(maps) == original, 'Game changed prepared maps'
            print(f'PASS {label}/{level}: H/Z preserve HUD; {len(route)} moves; VICTORY; Q=0; maps unchanged', flush=True)
        finally:
            prefix.with_suffix('.ansi.txt').write_text(''.join(game.raw), encoding='utf-8')
            prefix.with_suffix('.last-screen.txt').write_text(game.pump(), encoding='utf-8')
            if game.process.isalive():
                game.process.close(force=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--exe', required=True)
    parser.add_argument('--maps', required=True)
    parser.add_argument('--evidence', required=True)
    args = parser.parse_args()
    play(args.exe, args.maps, args.evidence, 'prepared')
