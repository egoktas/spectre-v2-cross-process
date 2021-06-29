#!/usr/bin/python3

# each char in the cache-channel is separated by 512 bytes
file_size=256*512

with open('shared_file', 'w') as f:
  for i in range(file_size//64):
    f.write( 63*'A' + '\n' )
