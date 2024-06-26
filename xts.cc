#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <openssl/evp.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <time.h>
using namespace std;
void print_hex(const unsigned char *data, int size){
  cout << hex;
  for (int i=0;i<size;++i) {
    if (i%30==0 && i) cout << endl;
    cout << setw(2) << setfill('0') << (int) data[i];
    }
  cout << dec << setfill(' ') << endl;
}
void print_with_commas(long number) {
  const int MAX=7;
  string numbers[MAX];
  char buffer[10];
  int last;
  cout << "\r";
  for (int i=0;i<MAX;++i) {
    sprintf(buffer,"%03d",abs(number)%1000);
    numbers[i]=string(buffer);
    if (abs(number)<1000) {
      sprintf(buffer,"%3d",number%1000);
      numbers[i]=string(buffer);
      last=i;
      break;
      }
    number/=1000;
    }
  for (int i=MAX-1;i>=0;--i) {
    cout << setw(4) << numbers[i];
    }
  cout << flush;
  }
void key_convert(char *str_key,unsigned char *key) {
int item[2]={};
for (int i=0;i<128;++i) {
  int ii=i%2;
  if (!str_key[i]) {
    if (ii) throw "Key too short";
    if (ii) key[i/2]=item[0]*16;
    break;
    }
  switch(str_key[i]) {
    case '0':  item[ii]=0;  break;
    case '1':  item[ii]=1;  break;
    case '2':  item[ii]=2;  break;
    case '3':  item[ii]=3;  break;
    case '4':  item[ii]=4;  break;
    case '5':  item[ii]=5;  break;
    case '6':  item[ii]=6;  break;
    case '7':  item[ii]=7;  break;
    case '8':  item[ii]=8;  break;
    case '9':  item[ii]=9;  break;
    case 'a':  item[ii]=10;
    case 'A':  item[ii]=10; break;
    case 'b':  item[ii]=11;
    case 'B':  item[ii]=11; break;
    case 'c':  item[ii]=12;
    case 'C':  item[ii]=12; break;
    case 'd':  item[ii]=13;
    case 'D':  item[ii]=13; break;
    case 'e':  item[ii]=14;
    case 'E':  item[ii]=14; break;
    case 'f':  item[ii]=15;
    case 'F':  item[ii]=15; break;
    default: throw "wrong key digit";
    }
  if (ii) {
    key[i/2]=item[0]*16+item[1];
    item[0]=0;
    item[1]=0;
    }
  }
}
EVP_CIPHER_CTX *context_d=EVP_CIPHER_CTX_new();
EVP_CIPHER_CTX *context_e=EVP_CIPHER_CTX_new();
unsigned char key1[64]={};
unsigned char key2[64]={};
unsigned char  iv[16]={};
const int BLOCK=512;
const int INLENGTH=BLOCK, BUFFER=32;
int OUTLENGTH=BLOCK;
//unsigned char inblock[INLENGTH]={},outblock[INLENGTH];
unsigned char inbuffer[BLOCK*BUFFER],outbuffer[BLOCK*BUFFER];
//int encr=-1;// unchanged
//int encr=0; // dencryption
//int encr=0; // decryption
int encr=0; // encryption
int decr=0; // decryption
bool zero=false;
ssize_t max_size=0;
//
//     MAIN:
//
int main(int argc, char ** argv) try {
if (argc<3) throw string("Invocation: ")+string(argv[0])+string(" filename hex-key1-512-bit [e|d|z|r] [key2|size if zero]\ne=encrypt with key1\nd=decrypt with key1\nz=encrypt autogenerated null blocks with key1\nr=\"recrypt\", decrypt with key1 and encrypt with key2");
if (argc==3) encr=1;
if (argc>=4) {
if (       string(argv[3])=="d") decr=1;
  else if (string(argv[3])=="e") encr=1;
  else if (string(argv[3])=="r") { encr=1; decr=1; }
  else if (string(argv[3])=="z") { zero=true; encr=1; }
  else throw string("third parameter if specified mut be [e|d|z|r]");
  }
if (zero) for (int i=0;i<INLENGTH*BUFFER;++i) inbuffer[i]=0;
int interval=5;
if (encr==1) cout << "Encrypting in " << interval << " seconds" << endl;
if (decr==1) cout << "Decrypting in " << interval << " seconds" << endl;
struct stat statbuf;
if (stat(argv[1],&statbuf)<0) {
  perror("stat");
  throw string("stat error");
  }
int fd;
if ((statbuf.st_mode&S_IFMT)==S_IFBLK) {
  fd=open(argv[1],O_RDWR|O_EXCL);
  if (fd<0) {
    perror("open");
    throw string("open error");
    }
  }
else if ((statbuf.st_mode&S_IFMT)==S_IFREG ) {
  fd=open(argv[1],O_RDWR);
  if (fd<0) {
    perror("open");
    throw string("open error");
    }
  }
else throw string("neither a device nor a regular file");
long &block=*reinterpret_cast<long*>(iv);
key_convert(argv[2],key1);
for (int i=0;argv[2][i];++i) argv[2][i]='X';
if (encr==1 && decr==1) {
  if (argc<5) throw "Second key missing";
  key_convert(argv[4],key2);
  for (int i=0;argv[4][i];++i) argv[4][i]='X';
  }
else memcpy(key2,key1,64);
cout << "Key(s):\n";
print_hex(key1,64);
print_hex(key2,64);
if (zero && argc>=5) {
  max_size=atol(argv[4]);
  cout << "Max size in 512-byte blocks: " << max_size << endl;
  }
if (fd<0) throw "No such file";
if (!EVP_CipherInit(context_e,EVP_aes_256_xts(),NULL,NULL,1)) throw "Error EVP_CipherInit 1e";
if (EVP_CIPHER_CTX_key_length(context_e)!=64) throw "key not 64*8?";
if (EVP_CIPHER_CTX_iv_length(context_e)!=16) throw "iv not 16*8?";
if (!EVP_CipherInit(context_d,EVP_aes_256_xts(),NULL,NULL,0)) throw "Error EVP_CipherInit 1d";
if (EVP_CIPHER_CTX_key_length(context_d)!=64) throw "key not 64*8?";
if (EVP_CIPHER_CTX_iv_length(context_d)!=16) throw "iv not 16*8?";
sleep(interval);
if (encr==1) cout << "Encrypting now"  << endl;
if (decr==1) cout << "Decrypting now"  << endl;
time_t tm=time(NULL),tm1;
time_t t0=time(NULL);
long iis=0;
long buf_index=0;
long start_iis=0;
long tail=0;
bool eof=false;
long x;
for (iis=0;;++iis) {
  buf_index=iis%BUFFER;
  if (buf_index==0) { //fill and dump the buffer
    if (iis!=0) { //dump the buffer
      int wrt=BUFFER;
      if (tail!=0) wrt=tail;
      if (pwrite(fd,outbuffer,BLOCK*wrt,start_iis*BLOCK)!=BLOCK*wrt) {
        cerr << "\nAttempted processing of block " << iis << endl;
        perror("pwrite");
        throw "Error pwrite, maybe EOF?";
        }
      }
    if (!zero && (x=pread(fd,inbuffer,BLOCK*BUFFER,iis*BLOCK))!=BLOCK*BUFFER) {
      cerr << "\nShort read. Attempted processing of block " << iis << endl;
      if (x==0) {
        cerr << "EOF?" << endl;
        break;
        }
      tail=x/BLOCK;
      cout << "tail is: " << tail << endl;
      if (x%BLOCK!=0) throw "programmer error 3";
      }
//  else if (tail>0) throw "programmer error 4";
    if (zero && max_size!=0) {
      if (tail) break;
      tail=max_size-iis;
      if (!tail) break;
      if (tail>=BUFFER) tail=0;
      else cout << "\n\ntail will be: " << tail << endl;
      }
    start_iis=iis;
    }
  if (iis%2048==0) {
    tm1=time(NULL);
    if (tm1!=tm) {
      tm=tm1;
      print_with_commas(iis*BLOCK);
      }
    }
  block=iis;
  int x;
  if (decr==1) {
    if (!EVP_CIPHER_CTX_reset(context_d)) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "Error EVP_CIPHER_CTX_reset";
      }
    if (!EVP_CipherInit(context_d,EVP_aes_256_xts(),key1,iv,0)) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "Error EVP_CipherInit 2";
      }
    if (!EVP_CipherUpdate(context_d,outbuffer+buf_index*BLOCK,&OUTLENGTH,
                                    inbuffer +buf_index*BLOCK,INLENGTH)) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "EVP_CipherUpdate";
      }
    if (OUTLENGTH!=INLENGTH) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "OUTLENGTH!=INLENGTH";
      }
    }
  if (decr==1 && encr==1) memcpy(inbuffer+buf_index*BLOCK,outbuffer+buf_index*BLOCK,INLENGTH);
  if (encr==1) {
    if (!EVP_CIPHER_CTX_reset(context_e)) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "Error EVP_CIPHER_CTX_reset";
      }
    if (!EVP_CipherInit(context_e,EVP_aes_256_xts(),key2,iv,1)) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "Error EVP_CipherInit 2";
      }
    if (!EVP_CipherUpdate(context_e,outbuffer+buf_index*BLOCK,&OUTLENGTH,inbuffer+buf_index*BLOCK,INLENGTH)) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "EVP_CipherUpdate";
      }
    if (OUTLENGTH!=INLENGTH) {
      cerr << "\nAttempted processing of block " << iis << endl;
      throw "OUTLENGTH!=INLENGTH";
      }
    }
//if (pwrite(fd,outblock,BLOCK,iis*BLOCK)!=BLOCK) {
//  cerr << "\nAttempted processing of block " << iis << endl;
//  perror("pwrite");
//  throw "Error pwrite, maybe EOF?";
//  }
  }
time_t t1=time(NULL);
cout << "Time in loop: " << t1-t0 << endl;
cout << "Speed: " << 1.0*iis*BLOCK/(t1-t0)/1024/1024 << " MB/s" << endl;
if (encr==1) {
  EVP_CipherFinal(context_e,outbuffer,&OUTLENGTH);
  if (OUTLENGTH!=0) throw "OUTLENGTH!=0";
  print_hex(outbuffer,OUTLENGTH);
  }
if (decr==1) {
  EVP_CipherFinal(context_d,outbuffer,&OUTLENGTH);
  if (OUTLENGTH!=0) throw "OUTLENGTH!=0";
  print_hex(outbuffer,OUTLENGTH);
  }
EVP_CIPHER_CTX_free(context_e);
EVP_CIPHER_CTX_free(context_d);
for (int i=0;i<64;++i) key1[i]=0;
for (int i=0;i<64;++i) key2[i]=0;
} catch (const char *x) {
//EVP_CipherFinal(context,outblock,&OUTLENGTH);
//if (OUTLENGTH!=0) cerr << "\nOUTLENGTH!=0 in error handler" << endl;
//print_hex(outblock,OUTLENGTH);
  EVP_CIPHER_CTX_free(context_e);
  EVP_CIPHER_CTX_free(context_d);
  for (int i=0;i<64;++i) key1[i]=0;
  for (int i=0;i<64;++i) key2[i]=0;
  cout << "\n" << x << endl;
  }
catch (const string & x) {
  cerr << x << endl;
  EVP_CIPHER_CTX_free(context_e);
  EVP_CIPHER_CTX_free(context_d);
  for (int i=0;i<64;++i) key1[i]=0;
  for (int i=0;i<64;++i) key2[i]=0;
  }
