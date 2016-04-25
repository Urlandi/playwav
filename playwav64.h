#define _PLAYWAV_64_H_

#define  EFOPEN -1
#define  EFREAD 11
#define   EHEAD -2
#define    EGEN 9
#define  ENOARG 3
#define EACSDND 4
#define ENUDATA 0

#define   MEHEAD "Unavailable .WAV"
#define    MEGEN "Error!"
#define   MEEXIT "Press any key to exit..."
#define MENUDATA "No data in file"

#define  MENOARG "Use next:\nplaywav <WAVFILE> [clock multiplier] [w] - play file <WAVFILE>\n\
:if w - show waveform graph only\n\
:clock multiplier - 650000 by default"

#define   RIFFID 0x46464952 //RIFF
#define   FORMAT 0x45564157 //WAVE
#define    FMTID 0x20746d66 //fmt
#define   DATAID 0x61746164 //data
#define   FACTID 0x74636166 //fact

#define PCMAUDIO 1 //PCM Audio

#define TOPHEADSIZE 44
#define SIZEOFEXTENSION 0

#define  FMTPCMSIZE 16		
#define	FMTFACTSIZE 18

#define  BODYCHAR '*'
#define   TOPCHAR '+'
#define ERASECHAR '-'
#define  ZEROCHAR '='

#define DEFHEIGHT 25
#define  DEFWIDTH 80
#define MAXHEIGHT 256
#define  MAXWIDTH 256
#define    STARTX 2
#define    STARTY 1

#define SPKPORT 0x61

#define TOPPRIORITY -20

#define ONESECOND 650000

#define    ARGPLAY 1
#define ARGSETTIME 2
#define ARGSHOWWAV 3


typedef struct {
	 int id;
	 int size;
	 int format;
} whriff;

typedef struct {
	 int id;
	 int size;
	short int audioformat;
	short int numchannels;
	 int samplerate;
	 int byterate;
	short int blockalign;
	short int bitspersample;
} whfmt;

typedef struct __attribute__((packed)) {
	short int sizeofext;
	 int id;
	 int size;
	 int samplelength;
} whfact;

typedef struct {
	 int id;
	 int size;
	 int samplelength;
} whfact2;


typedef struct {
	 int id;
	 int size;	
} whdata;

typedef struct {
	whriff riff;
	 whfmt fmt;
	whdata data;	
} wavpcm;

typedef struct __attribute__((packed)) {
	whriff riff;
	 whfmt fmt;
	whfact fact;
	whdata data;
} wavfact;

typedef struct {
	whriff riff;
	 whfmt fmt;
	whfact2 fact;
	whdata data;
} wavfact2;

typedef union {	
	wavpcm pcm;
	wavfact fact;
	wavfact2 fact2;
} wavhead;

int ShowErr(char,char*);
char TopHeadCheck(wavhead*,whriff**,whfmt**);
int DataHeadCheck(wavhead*,whriff*,whfmt*,whdata**,char);
int FileRead(FILE**,char*,int);
void WaveShow(char*,short int,short int,short int,short int);
void SetRawKbd(char);
char CheckArgs(short int);
short int InitIOPorts(char);
int InitWavFile(FILE**,char*,wavhead*,whfmt**);
