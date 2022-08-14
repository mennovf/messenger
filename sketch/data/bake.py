import json
from pathlib import Path
import struct

def utf8bytes(s):
    return eval('b"' + s.replace('\\u00', '\\x') + '"')

HEART = b'\xe2\x9d\xa4'

MTYPE_STR = bytes([0])
MTYPE_BMP = bytes([1])

ADDITIONAL_CHARS = set(map(lambda bs: ord(bs.decode('utf-8')), [HEART]))

chats = Path('.') / 'chat'

BAKE_PATH = Path('bake/')
BAKE_PATH.mkdir(exist_ok=True)

IMAGES_PATH = Path('images/')
img_mapping = {}
for i, img_path in enumerate(IMAGES_PATH.glob('*.bmp')):
    mapped_name = f'{i:08X}.bmp'
    img_mapping[img_path.stem] = mapped_name.encode("ascii")
    (BAKE_PATH / 'images' / mapped_name).open('wb').write(img_path.open('rb').read())

def img_content(s):
    return MTYPE_BMP, img_mapping[s[0]['uri'].split('/')[-1].split('.')[0]]

HIM = 'Menno Vanfrachem'
HER = 'Margarita Osipova'

for writer, reacter, foutname in ((HIM, HER, "him"), (HER, HIM, "her")):
    selection = []
    all_chars = set()
    for mfile in chats.glob('message_*.json'):
        with mfile.open() as f:
            text = f.read()
            text = text.replace('\\u', '\\\\u')
            o = json.loads(text)
            messages = o['messages']
            for m in messages:
                if m['sender_name'] != HIM:
                    continue

                if 'reactions' not in m:
                    continue
                reactions = m['reactions']
                for r in reactions:
                    if r['actor'] == HER and utf8bytes(r['reaction']) == HEART:
                        break
                else:
                    continue

                # Determine the type
                if 'content' in m:
                    mtype = MTYPE_STR
                    content = bytearray(m['content'].encode('utf-8'))
                    replace_sequences = []
                    for i, _ in enumerate(content[:-5]):
                        if content[i:i+4] == b'\\u00':
                            replace_sequences.append(content[i:i+6])
                    for rs in replace_sequences:
                        content = content.replace(rs, bytes.fromhex(rs[4:].decode('utf-8')))
                elif 'photos' in m:
                    mtype, content = img_content(m['photos'])
                elif 'videos' in m:
                    mtype, content = img_content(m['videos'])
                elif 'gifs' in m:
                    mtype, content = img_content(m['gifs'])
                else:
                    continue

                selection.append(mtype + content + b'\0')
                all_chars |= {ord(c) for c in content.decode('utf-8')}

    indices = [0]
    for s in selection:
        indices.append(len(s) + indices[-1])
    indices = indices[:-1] # Points into the void

    with (BAKE_PATH / foutname).open('wb') as f:
        f.write(struct.pack('>L', len(indices)))
        for index in indices:
            f.write(struct.pack('>L', index))
        for s in selection:
            f.write(s)

all_chars |= ADDITIONAL_CHARS
all_chars = {c for c in all_chars if c >= 32}
with open('codepoints.txt', 'w') as f:
    f.write(','.join(hex(c) for c in all_chars))

print(f'Stored {len(selection)} messages with {len(all_chars)} unique codepoints.')


