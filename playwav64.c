#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sched.h>
#include "playwav64.h"

void main (int argc, char *argv[]){
	FILE *fwav = NULL;	
	
	wavhead wh;
	whfmt *whft = NULL;
		
	char playmode = 0;
	
	unsigned int datasize = 0;
	
	char wavdata[0x10000];
	unsigned int *curdata = NULL;
	unsigned int bufsize = 0;
	char showwavdata[0x100];
	
	int i = 0, j = 0, k = 0;
	
	unsigned int cursampleraw = 0;
	unsigned int datamask = 0xFFFFFFFF;
	int cursample = 0;
	short int onechannelinc = 0;
	unsigned int samplezero = 0;
	char normalize_k = 8;
	
	short int pause = 0;
	int onesecond = ONESECOND;
	
	short int maxsample = DEFWIDTH-2;
	short int showsamplesize = (DEFHEIGHT-1)>>1;
	
	char kbh_ch = EOF;
	
	struct winsize scrsize;
	
	unsigned char out61 = 0;
	short int permit61 = -1;
	
	int errorscount = 0;
	
	if (argc < 2){
		ShowErr(ENOARG,NULL);
	}else if((playmode = CheckArgs(argc)) >= ARGSETTIME && !(onesecond=atol(argv[2])))
		onesecond = ONESECOND;
  
	if (playmode && (datasize = InitWavFile(&fwav,argv[1],&wh,&whft)) > 0){
				
		SetRawKbd(0);
	  
	        if ((ioctl(STDOUT_FILENO,TIOCGWINSZ,&scrsize) != -1)
	         && (scrsize.ws_col < MAXWIDTH && scrsize.ws_row < MAXHEIGHT)){
	   		maxsample = scrsize.ws_col-2;
	   		showsamplesize = (scrsize.ws_row-2)>>1;	   	
	   	}
                   
		if (playmode == ARGSHOWWAV) printf("\e[2J\e[%d;1H%s",(showsamplesize<<1)+2,MEEXIT);
	  	  
	  	onechannelinc = whft->numchannels*whft->bitspersample>>3;
	  	samplezero = 1<<(whft->bitspersample-1);
	  	pause = onesecond/whft->samplerate;
	  	
	  	if((whft->bitspersample>>3) < sizeof(cursampleraw)){
	  		datamask = ((1<<whft->bitspersample)-1);
	  		normalize_k = 0;
	  	}
	  	
	  	if(playmode < ARGSHOWWAV){ 
	  		if(permit61 = InitIOPorts(0))ShowErr(EACSDND,argv[0]);	  	 
	  	}else{
	  		permit61 = 0;
	  	}
	  	
//Start main read file		
	  while (!feof(fwav) && kbh_ch == EOF && bufsize >= 0 && datasize > 0 && !permit61){
	  	
	  	bufsize = FileRead(&fwav, wavdata, sizeof(wavdata)-sizeof(cursample));
	  	
//Start main play .wav data	  	
	  	for (i = 0;
	  	     i < bufsize && ((kbh_ch = getchar()) == EOF);
	  	     i += onechannelinc){
	  		
	  		curdata = (void*)(wavdata+i);
	  		cursampleraw = *curdata&datamask;
	  		
//Play in pc-speaker
	  		if (playmode < ARGSHOWWAV){	  		 
	  			if (cursampleraw > samplezero){
	  				out61 |= 0x2;	  				
	  			}else{
	  				out61 &= 0xFD;
	  			}
	  			for (k=0;k<pause;k++)outb(out61,SPKPORT);	  			
	  		}	
			
//Show waveform graph	  		
	  		if (playmode == ARGSHOWWAV){
	  			cursample = cursampleraw-samplezero;
	  			if ((showwavdata[j] = ((cursample>>normalize_k)*showsamplesize)>>(whft->bitspersample-normalize_k-2))&1){
					showwavdata[j]+=2;
				}
				showwavdata[j] = showwavdata[j]>>1;
	  			if (j++ == maxsample){
	  				WaveShow(showwavdata,STARTY,STARTX,showsamplesize,maxsample);
	  				j=0;
	  			}
	  			usleep(pause);
	  		}
	  	}
	  	datasize -= bufsize;
	  }
	  	if(bufsize < 0) ShowErr(EFREAD,argv[1]);	  	
	  	SetRawKbd(1);
	  	
	 } else ShowErr((char)datasize,argv[1]); 
	 
	 if ((playmode < ARGSHOWWAV) && permit61 == 0) InitIOPorts(1);
	 if (!errorscount && (playmode == ARGSHOWWAV)) printf("\e[2J\e[H");
	 
	 if (fwav) fclose(fwav);
}
	
int ShowErr(char err, char *merr){
	static int againerror = 0;
	if (!againerror)
	switch(err){
		case EFOPEN:
		case EFREAD:
		case EACSDND:
			    perror(merr);
			    break;
		case EHEAD: puts(MEHEAD);
			    break;
		case ENOARG: puts(MENOARG);
		     break;
		case ENUDATA: puts(MENUDATA);
		     break;
		default: puts(MEGEN);
	}
	return (againerror++);	
}	
char TopHeadCheck (wavhead *wh, whriff **wh_r, whfmt **wh_ft){
	char dataheadsize = -1;
	
	whriff *whr = *wh_r = &wh->pcm.riff;
	whfmt *whft = *wh_ft = &wh->pcm.fmt;
	whdata *whd = &wh->pcm.data;
	whfact *whf = &wh->fact.fact;
			
	if ((whr->id == RIFFID)
	 && (whr->format == FORMAT)
	 && (whft->id == FMTID)
	 && (whft->audioformat == PCMAUDIO)
	 && (whft->bitspersample <= 32)
	 && (whft->blockalign == whft->numchannels*whft->bitspersample>>3)
	 && (whft->byterate == whft->blockalign*whft->samplerate))
	 if (whft->size == FMTFACTSIZE && whf->sizeofext == SIZEOFEXTENSION){
	 	dataheadsize = sizeof(whfact);
	 }else if (whft->size == FMTPCMSIZE){
	 	if (whd->id == DATAID){
	 		dataheadsize = 0;
	 	}else if (whd->id == FACTID){
	 		dataheadsize = sizeof(whfact2);
		}
	 }		
	return(dataheadsize);
}

int DataHeadCheck(wavhead *wh, whriff *whr, whfmt *whft, whdata **wh_d, char dataheadsize){
	int datasize = 0;
	
	whdata *whd = NULL;
	
	switch (dataheadsize) {
		case 0:
			whd = &wh->pcm.data;
			break;
		case sizeof(whfact):
			whd = &wh->fact.data;
			break;
		case sizeof(whfact2):
			whd = &wh->fact2.data;
			break;
	}
	
	*wh_d = whd;
	
	if ((whd->id == DATAID)
	 && (whd->size < whr->size)){ 
	 	datasize=whd->size;
	}
	return(datasize);
}

int FileRead(FILE **fwav, char *wavdata, int readsize){
	int dataread = 0;	
	if(!feof(*fwav)){
		dataread = fread(wavdata,1,readsize,*fwav);
		if(ferror(*fwav)) dataread = -1;
	}
	return dataread;
}

void WaveShow(char *showwavdata, short int corny, short int cornx, short int showsamplesize, short int samplecount){
	int i = 0, j = 0;
	short int windowsize = showsamplesize<<1;
	
	for (j=0;j<=windowsize;j++){
	 printf("\e[%d;%dH",corny+j,cornx);
	 for (i=0;i<samplecount;i++){
	 	if (showwavdata[i] == (showsamplesize-j)){
			if (j == showsamplesize) putchar(ZEROCHAR); else putchar(TOPCHAR); 
		}else if ( (showwavdata[i] >= 0 && j <= showsamplesize && showwavdata[i] >= (showsamplesize-j))
		        || (showwavdata[i] <= 0 && j >= showsamplesize && showwavdata[i] <= (showsamplesize-j)) ){
			putchar(BODYCHAR);
		}else putchar(ERASECHAR);
	 }
	}
}
void SetRawKbd(char restore){
	static struct termios kbh_oldt;
	struct termios kbh_newt;
	static short int kbh_oldf = 0;
	
	if(restore){
		tcsetattr(STDIN_FILENO, TCSANOW, &kbh_oldt);
	  	fcntl(STDIN_FILENO, F_SETFL, kbh_oldf);
	}else{
		tcgetattr(STDIN_FILENO, &kbh_oldt);
  		kbh_newt = kbh_oldt;
  		kbh_newt.c_lflag &= ~(ICANON | ECHO);
  		tcsetattr(STDIN_FILENO, TCSANOW, &kbh_newt);
  		kbh_oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  		fcntl(STDIN_FILENO, F_SETFL, kbh_oldf | O_NONBLOCK);
  	}
}

short int InitIOPorts(char restore){
	short int permit61 = -1;
	static unsigned char old61 = 0;
	unsigned char out61 = 0;
//	struct sched_param schedio;
	
	short int r = 0;
	
	
	if (restore){
		outb(old61,SPKPORT);
	 	ioperm(SPKPORT,1,0);
	}else{
		if (!(permit61 = ioperm(SPKPORT,1,1))){
	  	 	old61 = inb(SPKPORT);
	  	 	out61 = old61 & 0xFE;
	  	 	outb(old61,SPKPORT);
	  	 	
	  	 	//nice(TOPPRIORITY);	  	 	
	  	 	
	  	 	//sched_getparam(0,&schedio);	  	 	
	  	 	//schedio.sched_priority = sched_get_priority_max(SCHED_FIFO);
	  	 	//r = sched_setscheduler(0,SCHED_FIFO,&schedio);	  	 	
	  	 }	
	}
		
	return (permit61);
}

char CheckArgs(short int argc){
	char playmode = ARGPLAY;
	
	if (argc == 3)playmode = ARGSETTIME;	
	if (argc > 3) playmode = ARGSHOWWAV;
	return (playmode);
}

int InitWavFile(FILE **fwav, char *filename, wavhead *wh, whfmt **wh_ft){
	char dataheadsize = -1;
	int datasize = 0;
	
	whdata *whd = NULL;
	whfmt *whft = NULL;
	whriff *whr = NULL;
	
do{
	if((*fwav = fopen(filename,"rb")) == NULL) {
		datasize = EFOPEN;
		break;
	}
	 
	if (*fwav && fread(wh,1,sizeof(whriff)+sizeof(whfmt)+sizeof(whdata),*fwav) == TOPHEADSIZE){
		dataheadsize = TopHeadCheck(wh, &whr, &whft);
		*wh_ft = whft;
	}else {
		datasize = EHEAD;
		break;
	}
	 
	if (dataheadsize >= 0 && fread(&((wh->fact2).fact.samplelength),1,dataheadsize,*fwav) == dataheadsize){
	 	datasize = DataHeadCheck(wh, whr, whft, &whd, dataheadsize);
	}else {
		datasize = EHEAD;
		break;
	}
	
}while(0);	

	return(datasize);
}
