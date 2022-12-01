# Hard drive encryption, decryption and zeroing using 512-bit XTS cipher.

This short program was written since openssl does not provide XTS
encoding from the command-line tool. The goal is to have a small handy
tool to encrypt existing hard drive partitions "in place", decrypt them
or change the encryption key. It also allows writing encrypted zeros to
the drive for either initialization purposes, testing or just scrubbing.

Invocation: `./xts filename hex-key1-512-bit [e|d|z|r] [key2]`

where:
+ filename - could be a file or a block device
+ hex-key1-512-bit: as the name says
+ encryption method:
	+ e=encrypt with key1
	+ d=decrypt with key1
	+ z=encrypt auto-generated null blocks with key1
	+ r="recrypt", decrypt with key1 and encrypt with key2
+ optional 512-bit XTS "key2" needed when "r" is specified.

Please note than in case of "r" no plaintext is written to the hard
drive, but it is immediately encrypted with the second key and a new
ciphertext is written.
