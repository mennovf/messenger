import json
from pathlib import Path

def utf8bytes(s):
    return eval('b"' + s.replace('\\u00', '\\x') + '"')

HEART = b'\xe2\x9d\xa4'

ADDITIONAL_CHARS = set(map(lambda bs: ord(bs.decode('utf-8')), [HEART]))

chats = Path('.') / 'chat'
selection = []
all_chars = set()
for mfile in chats.glob('message_*.json'):
    with mfile.open() as f:
        text = f.read()
        text = text.replace('\\u', '\\\\u')
        o = json.loads(text)
        messages = o['messages']
        for m in messages:
            if m['sender_name'] != 'Menno Vanfrachem':
                continue
            if 'content' not in m:
                continue
            if 'reactions' not in m:
                continue
            reactions = m['reactions']
            for r in reactions:
                if r['actor'] == 'Margarita Osipova' and utf8bytes(r['reaction']) == HEART:
                    break
            else:
                continue

            content = bytearray(m['content'].encode('utf-8'))
            replace_sequences = []
            for i, _ in enumerate(content[:-5]):
                if content[i:i+4] == b'\\u00':
                    replace_sequences.append(content[i:i+6])
            for rs in replace_sequences:
                content = content.replace(rs, bytes.fromhex(rs[4:].decode('utf-8')))
            selection.append(content + b'\0')
            all_chars |= {ord(c) for c in content.decode('utf-8')}

indices = [0]
for s in selection:
    indices.append(len(s) + indices[-1])

with open('loved.h', 'w') as f:
    f.write('#include <stdint.h>\n\n')
    f.write('uint8_t const LOVED[] PROGMEM = {')
    f.write(','.join(hex(c) for s in selection for c in s))
    f.write('};\n')
    f.write(f'uint32_t const LOVED_INDICES[{len(indices)}] PROGMEM = {{')
    f.write(','.join(str(i) for i in indices))
    f.write('};\n')

all_chars |= ADDITIONAL_CHARS
all_chars = {c for c in all_chars if c >= 32 and c < 30000}
with open('codepoints.txt', 'w') as f:
    f.write(','.join(hex(c) for c in all_chars))

print(f'Stored {len(selection)} messages with {len(all_chars)} unique codepoints.')


