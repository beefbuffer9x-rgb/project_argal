# project_argal
a vibecoded code virtualizer for 64-bit executables with a region modifying anti tamper check
## DISCLAIMER
this project is made using claude sonnet 4.6, this project is a base for people looking forward to learning reversing.
this project is constantly going to be updated with new and improved functions etc, this could be a good start for learning or analyzing the framework of a simple code virtualizer
dont forget, its vibecoded.

## cli usage(engine):
obfuscator.exe example.exe payload.dll out.exe 0x1000 0x1021
0x1000 = start rva address of a function
0x1021 = end rva address of a function
payload dll must be in the same folder
