/*
** Example Winamp .RAW input plug-in
** Copyright (c) 1998, Justin Frankel/Nullsoft Inc.
*/

#include <windows.h>
#include <windowsx.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <string.h>
#include <stdio.h>


//#include "FileDlg.h"
//#include "Reg.h"
//#include "WinResUtil.h"

//#include "./VBA/GBA.h"
//#include "./VBA/Globals.h"
//#include "./VBA/Sound.h"
//#include "./VBA/Util.h"

//#include "./VBA/win32/VBA.h"
#include "resource.h"
#include "gsf.h"

#include "in2.h"

#define ProgName "Highly Advanced"
#define ProgVer  " v0.02"
#define AppVer   ProgName ProgVer " by Zoopd and CaitSith2.\nBased on the Visual Boy Advance code.\nOriginal PSF concept by Neill Corlett."
//#define AppName  ProgName " (x86)"
#define AppReg   "Software\\Zoopd and Caitsith2\\" ProgName

short sample_buffer[576*2*2]; // used for DSP

#define WINAMP_BUFFER_SIZE (576*4)


// avoid CRT. Evil. Big. Bloated.
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

// post this to the main window at end of file (after playback as stopped)
#define WM_WA_MPEG_EOF WM_USER+2

// raw configuration
//#define NCH 2
//#define SAMPLERATE 44100
//#define BPS 16

int TrackLength=0;
int FadeLength=0;

int IgnoreTrackLength=0, DefaultLength=1;
int deflen=120,deffade=4,silencelength=5,silencedetected=0;
int DetectSilence=0;
char titlefmt[256]="%game% - %title%";

char *infoDlgpsftag=NULL;
char *getfileinfopsftag=NULL;
int infoDlgWriteNewTags=0;


int sndBitsPerSample;
int sndSamplesPerSec;
int sndNumChannels;

unsigned char endofseek = 0;

extern unsigned short soundFinalWave[1470];
extern int soundBufferLen;

extern char soundEcho;
extern char soundLowPass;
extern char soundReverse;
extern char soundQuality;

In_Module mod; // the output module (declared near the bottom of this file)
char lastfn[MAX_PATH]; // currently playing file (used for getting info on the current file)
unsigned char IsCurrentFile = FALSE;
int file_length; // file length, in bytes
double decode_pos_ms; // current decoding position, in milliseconds
int paused; // are we paused?
int seek_needed; // if != -1, it is the point that the decode thread should seek to, in ms.
//char sample_buffer[576*NCH*(BPS/8)*2]; // sample buffer

HANDLE input_file=INVALID_HANDLE_VALUE; // input file handle

int killDecodeThread=0;					// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread

DWORD WINAPI __stdcall DecodeThread(void *b); // the decode thread procedure
BOOL CALLBACK configDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int infoDlg(char *fn, HWND hwnd);
void ReadSettings(void);
void WriteSettings(void);

int track_length;



void GetFile(char *filename, char *buffer)
{
	char *p = strrchr(filename, '\\');

	if(p)
		strcpy(buffer,p+1);
}

void config(HWND hwndParent)
{
	DialogBox(mod.hDllInstance, (const char *)IDD_CONFIG, hwndParent, configDlgProc);
}

void about(HWND hwndParent)
{
	MessageBox(hwndParent,AppVer,"About",MB_OK);
}

void init() { srand(time(NULL)); ReadSettings(); }

void quit() { WriteSettings();}

int isourfile(char *fn) { return 0; } 
// used for detecting URL streams.. unused here. strncmp(fn,"http://",7) to detect HTTP streams, etc

int play(char *fn) 
{ 
	int maxlatency;
	int thread_id;

	input_file = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (input_file == INVALID_HANDLE_VALUE) // error opening file
	{
		return 1;
	}
	file_length=GetFileSize(input_file,NULL);

	strcpy(lastfn,fn);
	paused=0;
	decode_pos_ms=0;
	seek_needed=-1;

	GSFRun(fn);

	maxlatency = mod.outMod->Open(sndSamplesPerSec,sndNumChannels,sndBitsPerSample, -1,-1);
	if (maxlatency < 0) // error opening device
	{
		CloseHandle(input_file);
		input_file=INVALID_HANDLE_VALUE;
		return 1;
	}
	// dividing by 1000 for the first parameter of setinfo makes it
	// display 'H'... for hundred.. i.e. 14H Kbps.
	mod.SetInfo(sndBitsPerSample,sndSamplesPerSec/1000,sndNumChannels,1);

	// initialize vis stuff
	mod.SAVSAInit(maxlatency,sndSamplesPerSec);
	mod.VSASetInfo(sndSamplesPerSec,sndNumChannels);

	mod.outMod->SetVolume(-666); // set the output plug-ins default volume

	killDecodeThread=0;
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) DecodeThread,(void *) &killDecodeThread,0,&thread_id);
	
	return 0; 
}

void pause() { paused=1; mod.outMod->Pause(1); }
void unpause() { paused=0; mod.outMod->Pause(0); }
int ispaused() { return paused; }

void stop() { 
	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		if (paused)
			unpause();
		killDecodeThread=1;
		if (WaitForSingleObject(thread_handle,5000) == WAIT_TIMEOUT)
		{
			MessageBox(mod.hMainWindow,"error asking thread to die!\n","error killing decode thread",0);
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}
	if (input_file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(input_file);
		input_file=INVALID_HANDLE_VALUE;
	}
	GSFClose();

	mod.outMod->Close();

	mod.SAVSADeInit();
}

void end_of_track()			//wrapper, since stop is a variable in the emulator code
{
	PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
	ExitThread(0);
}

int getlength() {
	return track_length;
	//return (file_length*10)/(sndSamplesPerSec/100*sndNumChannels*(sndBitsPerSample/8)); 
}

int getoutputtime() { 
	return decode_pos_ms+(mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime()); 
}

void setoutputtime(int time_in_ms) { 
	seek_needed=time_in_ms; 
}

void setvolume(int volume) { mod.outMod->SetVolume(volume); }
void setpan(int pan) { mod.outMod->SetPan(pan); }


extern int LengthFromString(const char * timestring);

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	DWORD dwRead;
	DWORD FileSize;
	DWORD tagstart;
	DWORD taglen;
	DWORD reservesize;
	HANDLE hFile;
	BYTE buffer[256],length[256],fade[256];
	BYTE Test[5];
	//char *psftag;

	if (!filename || !*filename) filename=lastfn;
	//if (title) sprintf(title,"%s",filename);

	if(!strcmp(filename,lastfn))
		IsCurrentFile = TRUE;
	else
		IsCurrentFile = FALSE;



	hFile = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		return;
	}

	ReadFile(hFile,Test,4,&dwRead,NULL);
	if (!IsValidGSF(Test)) {
		CloseHandle(hFile);
		return;
	}
	// Read GSF contents
	ReadFile(hFile,&reservesize,4,&dwRead,NULL); // size of reserved area
	ReadFile(hFile,&FileSize,4,&dwRead,NULL); // size of program
	ReadFile(hFile,Test,4,&dwRead,NULL); // CRC (no check yet)
	
	tagstart=SetFilePointer(hFile,0,0,FILE_CURRENT)+FileSize+reservesize;
	SetFilePointer(hFile,tagstart,0,FILE_BEGIN);

	ReadFile(hFile,Test,5,&dwRead,NULL);
	// read tags if there are any
	if (IsTagPresent(Test)) {
		tagstart+=5;
		taglen=SetFilePointer(hFile,0,0,FILE_END)-tagstart;
		SetFilePointer(hFile,tagstart,0,FILE_BEGIN);
		getfileinfopsftag=malloc(taglen+2);
				
		ReadFile(hFile,getfileinfopsftag,taglen,&dwRead,NULL);
		getfileinfopsftag[taglen]='\0';

		psftag_raw_getvar(getfileinfopsftag,"length",length,sizeof(length)-1);
		psftag_raw_getvar(getfileinfopsftag,"fade",fade,sizeof(fade)-1);

		if (length_in_ms) *length_in_ms=LengthFromString(length)+LengthFromString(fade);
		if (IgnoreTrackLength) *length_in_ms=0;

		// create title from PSF tag
		if (title) {
			DWORD c=0,tagnamestart;
			char tempchar;
			sprintf(title,"");
			while (c < strlen(titlefmt)) {
				if (titlefmt[c]=='%') {
					tagnamestart=++c;
					for (;c<strlen(titlefmt) && titlefmt[c] != '%'; c++);
					if (c == strlen(titlefmt)) {
						DisplayError("Bad title format string (unterminated token)");
						return;
					}
					titlefmt[c]='\0';
					if(!strcmp(titlefmt+tagnamestart,"file"))
						GetFile(filename,buffer);
					else
						psftag_raw_getvar(getfileinfopsftag,titlefmt+tagnamestart,buffer,sizeof(buffer)-1);
					strcat(title,buffer);
					titlefmt[c]='%';
				} else {
					tempchar=titlefmt[c+1];
					titlefmt[c+1]='\0';
					strcat(title,titlefmt+c);
					titlefmt[c+1]=tempchar;
				}
				c++;
			}
		}

		free(getfileinfopsftag);
		getfileinfopsftag=NULL;
	}
	else
	{
		if (title)
		{
			GetFile(filename,buffer);
			sprintf(title,"");
			strcat(title,buffer);
		}
	}
	if (DefaultLength && *length_in_ms <= 0) *length_in_ms=(deflen+deffade)*1000;
	CloseHandle(hFile);
	if(IsCurrentFile)
		track_length = *length_in_ms;
}



void eq_set(int on, char data[10], int preamp) 
{ 
	// most plug-ins can't even do an EQ anyhow.. I'm working on writing
	// a generic PCM EQ, but it looks like it'll be a little too CPU 
	// consuming to be useful :)
}


// render 576 samples into buf. 
// note that if you adjust the size of sample_buffer, for say, 1024
// sample blocks, it will still work, but some of the visualization 
// might not look as good as it could. Stick with 576 sample blocks
// if you can, and have an additional auxiliary (overflow) buffer if 
// necessary.. 


DWORD WINAPI __stdcall DecodeThread(void *b)
{
	int done=0;
	while (! *((int *)b) ) 
	{
		/*if (seek_needed != -1)
		{
			int offs;
			decode_pos_ms = seek_needed-(seek_needed%1000);
			seek_needed=-1;
			done=0;
			mod.outMod->Flush(decode_pos_ms);
			offs = (decode_pos_ms/1000) * SAMPLERATE;
			SetFilePointer(input_file,offs*NCH*(BPS/8),NULL,FILE_BEGIN);
		}
		if (done)
		{
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying())
			{
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				return 0;
			}
			Sleep(10);
		}
		else if (mod.outMod->CanWrite() >= ((576*NCH*(BPS/8))<<(mod.dsp_isactive()?1:0)))
		{	
			int l=576*NCH*(BPS/8);
			l=get_576_samples(sample_buffer);
			if (!l) 
			{
				done=1;
			}
			else
			{
				mod.SAAddPCMData((char *)sample_buffer,NCH,BPS,decode_pos_ms);
				mod.VSAAddPCMData((char *)sample_buffer,NCH,BPS,decode_pos_ms);
				decode_pos_ms+=(576*1000)/SAMPLERATE;
				if (mod.dsp_isactive()) l=mod.dsp_dosamples((short *)sample_buffer,l/NCH/(BPS/8),BPS,NCH,SAMPLERATE)*(NCH*(BPS/8));
				mod.outMod->Write(sample_buffer,l);
			}
		}

		else*/ 

		//while (mod->outMod->CanWrite() < (remainingBytes/WINAMP_BUFFER_SIZE)*WINAMP_BUFFER_SIZE)

		EmulationLoop();
		//else
		//	Sleep(50);
	}
	return 0;
}



In_Module mod = 
{
	IN_VER,
	ProgName ProgVer,
	0,	// hMainWindow
	0,  // hDllInstance
	"gsf\0GSF File (*.gsf)\0minigsf\0Mini-GSF File (*.minigsf)\0",
	1,	// is_seekable
	1, // uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // vis stuff


	0,0, // dsp

	eq_set,

	NULL,		// setinfo

	0 // out_mod

};

__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}
int buffertime = 0;

void writeSound(void)
{
	int ret = soundBufferLen;
	int i;
	
	while (mod.outMod->CanWrite() < ((ret*sndNumChannels*(sndBitsPerSample/8))<<(mod.dsp_isactive()?1:0)))
		Sleep(50);


	mod.SAAddPCMData((char *)soundFinalWave,sndNumChannels,sndBitsPerSample,decode_pos_ms);
	mod.VSAAddPCMData((char *)soundFinalWave,sndNumChannels,sndBitsPerSample,decode_pos_ms);
	decode_pos_ms += (ret/(2*sndNumChannels) * 1000) / (float)sndSamplesPerSec;

	if (mod.dsp_isactive())
		ret=mod.dsp_dosamples((short *)soundFinalWave,ret/sndNumChannels/(sndBitsPerSample/8),sndBitsPerSample,sndNumChannels,sndSamplesPerSec)*(sndNumChannels*(sndBitsPerSample/8));
	


	//if(soundFinalWave[0]==0&&soundFinalWave[1]==0&&soundFinalWave[2]==0&&soundFinalWave[3]==0)
	//  DisplayError("%.2X%.2X%.2X%.2X - %d", soundFinalWave[0],soundFinalWave[1],soundFinalWave[2],soundFinalWave[3],ret);
	mod.outMod->Write(soundFinalWave,ret);
	

	if (seek_needed != -1)		//if a seek is initiated
	{
		mod.outMod->Flush((long)decode_pos_ms);
		
		if (seek_needed < decode_pos_ms)	//if we are asked to seek backwards.  we have to start from the beginning
		{
			GSFClose();
			GSFRun(lastfn);
			decode_pos_ms = 0;
		}
	}
}





BOOL CALLBACK rawDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		return 0;
	case WM_INITDIALOG:
		{
			char *LFpsftag;
			//char LFpsftag[500001];
			DWORD c,d;

			LFpsftag = malloc(strlen(infoDlgpsftag)*3+1);

			// <= includes terminating null
			for (c=0, d=0; c <= strlen(infoDlgpsftag); c++, d++) {
				LFpsftag[d]=infoDlgpsftag[c];
				if (infoDlgpsftag[c]=='\n') {
					LFpsftag[d++]='\r';
					LFpsftag[d]='\r';
					LFpsftag[d]='\n';
				}
			}
		
			SetDlgItemText(hDlg,IDC_RAWTAG,LFpsftag);
			//free(LFpsftag);
			break;
		}
	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
		case IDOK:
			EndDialog(hDlg,TRUE);
			break;
		}
	default:
		return 0;
	}
	return 1;
}

unsigned char pimpbot = FALSE;
unsigned char PepperisaGoodBoy = TRUE;


BOOL CALLBACK infoDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char Buffer[1024];
	char *LFpsftag;
	//char LFpsftag[50001];
	DWORD c,d;
	HANDLE surpriseImage;
	int randNum;


	switch (uMsg) {	
	case WM_CLOSE:
		if(decode_pos_ms  >= TrackLength-FadeLength)
				TrackLength=decode_pos_ms+FadeLength;
		EndDialog(hDlg,TRUE);
		return 0;
	case WM_INITDIALOG:
		pimpbot = FALSE;
		randNum = rand();
		if (PepperisaGoodBoy)
		{
		    
			if (randNum < RAND_MAX*0.025)	//  1/10th a chance of occurring
			{
				surpriseImage = LoadImage(mod.hDllInstance, MAKEINTRESOURCE(IDB_BIGKITTY), IMAGE_BITMAP, 0, 0, 0);		//change the logo graphic
				SendMessage( GetDlgItem(hDlg, IDC_LOGO), STM_SETIMAGE, IMAGE_BITMAP, (long)surpriseImage); //''
			}
			else if (randNum >= RAND_MAX*0.025 && randNum < RAND_MAX*0.05)
			{
				surpriseImage = LoadImage(mod.hDllInstance, MAKEINTRESOURCE(IDB_PIMPBOT), IMAGE_BITMAP, 0, 0, 0);		//change the logo graphic
				SendMessage( GetDlgItem(hDlg, IDC_LOGO), STM_SETIMAGE, IMAGE_BITMAP, (long)surpriseImage); //''
				pimpbot = TRUE;
			}
			else if (randNum >= RAND_MAX*0.05 && randNum < RAND_MAX*0.1)
			{
				surpriseImage = LoadImage(mod.hDllInstance, MAKEINTRESOURCE(IDB_PEPPER), IMAGE_BITMAP, 0, 0, 0);
				SendMessage( GetDlgItem(hDlg, IDC_LOGO), STM_SETIMAGE, IMAGE_BITMAP, (long)surpriseImage); //''
			}
		}
		psftag_raw_getvar(infoDlgpsftag,"title",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_TITLE,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"artist",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_ARTIST,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"game",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_GAME,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"year",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_YEAR,Buffer);
		
		psftag_raw_getvar(infoDlgpsftag,"copyright",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_COPYRIGHT,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"gsfby",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_GSFBY,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"length",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_LENGTH,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"fade",Buffer,sizeof(Buffer)-1);
		SetDlgItemText(hDlg,IDC_FADE,Buffer);

		psftag_raw_getvar(infoDlgpsftag,"comment",Buffer,sizeof(Buffer)-1);

		LFpsftag = malloc(strlen(Buffer)*3+1);

		for (c=0, d=0; c <= strlen(Buffer); c++, d++) {
			LFpsftag[d]=Buffer[c];
			if (Buffer[c]=='\n') {
				LFpsftag[d++]='\r';
				LFpsftag[d]='\r';
				LFpsftag[d]='\n';
			}
		}

		SetDlgItemText(hDlg,IDC_COMMENT,LFpsftag);
		free(LFpsftag);
		break;

	case WM_LBUTTONDOWN:
		if (pimpbot)
		{
			if (rand() > RAND_MAX/2) PlaySound((LPCSTR)IDR_PIMPBOT1, mod.hDllInstance, SND_RESOURCE|SND_ASYNC);
			else PlaySound((LPCSTR)IDR_PIMPBOT2, mod.hDllInstance, SND_RESOURCE|SND_ASYNC);
		}
		break;

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
		case IDOK:
			GetDlgItemText(hDlg,IDC_TITLE,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"title",Buffer);
			
			GetDlgItemText(hDlg,IDC_ARTIST,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"artist",Buffer);
				
			GetDlgItemText(hDlg,IDC_GAME,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"game",Buffer);

			GetDlgItemText(hDlg,IDC_YEAR,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"year",Buffer);
			
			GetDlgItemText(hDlg,IDC_COPYRIGHT,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"copyright",Buffer);

			GetDlgItemText(hDlg,IDC_GSFBY,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"gsfby",Buffer);

			GetDlgItemText(hDlg,IDC_LENGTH,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"length",Buffer);

			if(IsCurrentFile)
				TrackLength=LengthFromString(Buffer);
			
			GetDlgItemText(hDlg,IDC_FADE,Buffer,1024);
			psftag_raw_setvar(infoDlgpsftag,50000-1,"fade",Buffer);
			if(IsCurrentFile)
			{
				FadeLength=LengthFromString(Buffer);
				TrackLength+=FadeLength;
			

				if(TrackLength == 0)
				{
					TrackLength = (deflen + deffade)*1000;
					FadeLength = deffade*1000;
				}
				track_length=TrackLength;
			}



			//mod.GetLength;

			GetDlgItemText(hDlg,IDC_COMMENT,Buffer,1024);

			// remove 0x0d (in PSF a newline is 0x0a)
			for (c=0,d=0; c < strlen(Buffer); c++) {
				if (Buffer[c] != 0x0d) {
					Buffer[d]=Buffer[c];
					d++;
				}
			}
			Buffer[d]='\0';

			psftag_raw_setvar(infoDlgpsftag,50000,"comment",Buffer);

			infoDlgWriteNewTags=1;

			if(decode_pos_ms  >= TrackLength-FadeLength)
				TrackLength=decode_pos_ms+FadeLength;

			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			if(decode_pos_ms  >= TrackLength-FadeLength)
				TrackLength=decode_pos_ms+FadeLength;
			EndDialog(hDlg, TRUE);
			break;
		case IDC_NOWBUT:
			if(decode_pos_ms != 0)
			{
				sprintf(Buffer,"%i:%02i.%i",getoutputtime()/60000,(getoutputtime()%60000)/1000,getoutputtime()%1000);
				SetDlgItemText(hDlg,IDC_LENGTH,Buffer);
			}
			break;

		//case IDC_SECRETBUTTON:
		//	PlaySound((LPCSTR)IDR_WAVE1, mod.hDllInstance, SND_RESOURCE|SND_ASYNC);
		//	surpriseImage = LoadImage(mod.hDllInstance, MAKEINTRESOURCE(IDB_BIGKITTY), IMAGE_BITMAP, 0, 0, 0);		//change the logo graphic
			//SendMessage( GetDlgItem(hDlg, IDC_SECRETBUTTON), STM_SETIMAGE, IMAGE_BITMAP, (long)surpriseImage); //''
		//	SendMessage(GetDlgItem(hDlg,IDC_SECRETBUTTON),BM_SETIMAGE,IMAGE_BITMAP, (long)surpriseImage);
		//	break;
		case IDC_LAUNCHCONFIG:
			DialogBox(mod.hDllInstance, (const char *)IDD_CONFIG, hDlg, configDlgProc);
			break;
		case IDC_VIEWRAW:
			DialogBox(mod.hDllInstance, (const char *)IDD_RAWTAG, hDlg, rawDlgProc);
		}
		break;
	default:
		return 0;
	}

	return 1;
}


int playforever=0;
int infoDlg(char *fn, HWND hwnd)
{
	playforever=1;
//	sprintf(infoDlgfn,"Info for %s",fn);
	infoDlgpsftag=malloc(50001); //malloc(taglen+2); // for safety when editing
	infoDlgpsftag[0]='\0';
	psftag_readfromfile(infoDlgpsftag,fn);

	if(!strcmp(fn,lastfn))
		IsCurrentFile = TRUE;
	else
		IsCurrentFile = FALSE;
		
	infoDlgWriteNewTags=0;

	DialogBox(mod.hDllInstance, (const char *)IDD_GSFINFO, hwnd, infoDlgProc);
		
	if (infoDlgWriteNewTags) {
		psftag_writetofile(infoDlgpsftag,fn);
	}
	free(infoDlgpsftag);
	infoDlgpsftag=NULL;
	playforever=0;
	return 0;
}



BOOL CALLBACK configDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int mypri;
	char buf[256];
	HANDLE hSlider;
	switch (uMsg) {	
	case WM_CLOSE:
		EndDialog(hDlg,TRUE);
		return 0;
	case WM_INITDIALOG:
		if (soundEcho) CheckDlgButton(hDlg,IDC_ECHO,BST_CHECKED);
		if (soundLowPass) CheckDlgButton(hDlg,IDC_LOWPASSFILTER,BST_CHECKED);
		if (soundReverse) CheckDlgButton(hDlg,IDC_REVERSESTEREO,BST_CHECKED);
		if (IgnoreTrackLength) CheckDlgButton(hDlg,IDC_PLAYFOREVER,BST_CHECKED);
		if (DefaultLength) CheckDlgButton(hDlg,IDC_DEFLEN,BST_CHECKED);
		//if (IgnoreTrackLength) CheckDlgButton(hDlg,IDC_NOLENGTH,BST_CHECKED);
		//if (soundQuality == 1) CheckDlgButton(hDlg,IDC_44KHZ,BST_CHECKED);
		//if (soundQuality == 2) CheckDlgButton(hDlg,IDC_22KHZ,BST_CHECKED);
		
		if (DetectSilence) CheckDlgButton(hDlg,IDC_DETSIL,BST_CHECKED);
		//if (UseRecompiler) CheckDlgButton(hDlg,IDC_RECOMPILER,BST_CHECKED);
		SetDlgItemText(hDlg,IDC_TITLEFORMAT,titlefmt);
		sprintf(buf,"%i",deflen);
		SetDlgItemText(hDlg,IDC_DEFLENVAL,buf);
		sprintf(buf,"%i",deffade);
		SetDlgItemText(hDlg,IDC_DEFFADEVAL,buf);
		sprintf(buf,"%i",silencelength);
		SetDlgItemText(hDlg,IDC_DETSILVAL,buf);
		
		// set CPU Priority slider
		//hSlider=GetDlgItem(hDlg,IDC_PRISLIDER);
		//SendMessage(hSlider, TBM_SETRANGE, (WPARAM) TRUE,                   // redraw flag 
		//	(LPARAM) MAKELONG(1, 7));  // min. & max. positions 
		//SendMessage(hSlider, TBM_SETPOS, 
        //(WPARAM) TRUE,                   // redraw flag 
        //(LPARAM) CPUPriority+1);
		//mypri=CPUPriority;
		//SetDlgItemText(hDlg,IDC_CPUPRI,pristr[CPUPriority]);
		
		break;
	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
		case IDOK:
			soundEcho=(IsDlgButtonChecked(hDlg, IDC_ECHO) == BST_CHECKED) ? TRUE : FALSE;
			soundLowPass=(IsDlgButtonChecked(hDlg, IDC_LOWPASSFILTER) == BST_CHECKED) ? TRUE : FALSE;
			soundReverse=(IsDlgButtonChecked(hDlg, IDC_REVERSESTEREO) == BST_CHECKED) ? TRUE : FALSE;
			IgnoreTrackLength=(IsDlgButtonChecked(hDlg, IDC_PLAYFOREVER) == BST_CHECKED) ? TRUE : FALSE;
			DefaultLength=(IsDlgButtonChecked(hDlg,IDC_DEFLEN) == BST_CHECKED) ? TRUE : FALSE;
			DetectSilence=(IsDlgButtonChecked(hDlg,IDC_DETSIL) == BST_CHECKED) ? TRUE : FALSE;
			//if ((IsDlgButtonChecked(hDlg, IDC_44KHZ) == BST_CHECKED) ? TRUE : FALSE)
			//	soundQuality = 1;
			//if ((IsDlgButtonChecked(hDlg, IDC_22KHZ) == BST_CHECKED) ? TRUE : FALSE)
			//	soundQuality = 2;
			GetDlgItemText(hDlg,IDC_TITLEFORMAT,titlefmt,sizeof(titlefmt)-1);
			
			GetDlgItemText(hDlg,IDC_DEFLENVAL,buf,sizeof(buf)-1);
			sscanf(buf,"%i",&deflen);
			GetDlgItemText(hDlg,IDC_DEFFADEVAL,buf,sizeof(buf)-1);
			sscanf(buf,"%i",&deffade);
			GetDlgItemText(hDlg,IDC_DETSILVAL,buf,sizeof(buf)-1);
			sscanf(buf,"%i",&silencelength);
			silencedetected=0;
			//CPUPriority=mypri;
			WriteSettings();
			if(decode_pos_ms  >= TrackLength-FadeLength)
				TrackLength=decode_pos_ms+FadeLength;
		case IDCANCEL:
			EndDialog(hDlg,TRUE);
			break;
		}
/*	case WM_HSCROLL:
		if (lParam==GetDlgItem(hDlg,IDC_PRISLIDER)) {
			//DisplayError("HScroll=%i",HIWORD(wParam));
			
			if (LOWORD(wParam)==TB_THUMBPOSITION || LOWORD(wParam)==TB_THUMBTRACK) mypri=HIWORD(wParam)-1;
			else mypri=SendMessage(GetDlgItem(hDlg,IDC_PRISLIDER),TBM_GETPOS,0,0)-1;
			SetDlgItemText(hDlg,IDC_CPUPRI,pristr[mypri]);
		}
		break;
*/	default:
		return 0;
	}

	return 1;
}



void WriteSettings(void) {
	HKEY hKey;
	DWORD dwDisp;

	RegCreateKeyEx(HKEY_CURRENT_USER,AppReg,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp);
	RegSetValueEx(hKey,"Title Format",0,REG_SZ,titlefmt,strlen(titlefmt));
	RegSetValueEx(hKey,"Echo",0,REG_DWORD,(BYTE*)&soundEcho,sizeof(soundEcho));
	RegSetValueEx(hKey,"Low-pass Filter",0,REG_DWORD,(BYTE*)&soundLowPass,sizeof(soundLowPass));
	RegSetValueEx(hKey,"Reverse Stereo",0,REG_DWORD,(BYTE*)&soundReverse,sizeof(soundReverse));
	RegSetValueEx(hKey,"Ignore Track Limits",0,REG_DWORD,(BYTE*)&IgnoreTrackLength,sizeof(IgnoreTrackLength));
	//RegSetValueEx(hKey,"Sample Rate",0,REG_DWORD,(BYTE*)&soundQuality,sizeof(soundQuality));
	RegSetValueEx(hKey,"Pepper is a Good Boy",0,REG_DWORD,(BYTE*)&PepperisaGoodBoy,sizeof(PepperisaGoodBoy));
	
	//RegSetValueEx(hKey,"Audio HLE",0,REG_DWORD,(BYTE*)&AudioHLE,sizeof(AudioHLE));
	
	RegSetValueEx(hKey,"Default Length",0,REG_DWORD,(BYTE*)&DefaultLength,sizeof(DefaultLength));
	RegSetValueEx(hKey,"Default Length Value",0,REG_DWORD,(BYTE*)&deflen,sizeof(deflen));
	RegSetValueEx(hKey,"Default Fade Value",0,REG_DWORD,(BYTE*)&deffade,sizeof(deflen));
	RegSetValueEx(hKey,"Detect Silence",0,REG_DWORD,(BYTE*)&DetectSilence,sizeof(DetectSilence));
	RegSetValueEx(hKey,"Detect Silence Value",0,REG_DWORD,(BYTE*)&silencelength,sizeof(silencelength));
	//RegSetValueEx(hKey,"Use Recompiler",0,REG_DWORD,(BYTE*)&UseRecompiler,sizeof(UseRecompiler));
	//RegSetValueEx(hKey,"CPU Thread Priority",0,REG_DWORD,(BYTE*)&CPUPriority,sizeof(CPUPriority));

	RegCloseKey(hKey);
}

void ReadSettings(void) {
	HKEY hKey;
	long lResult, plResult;
	unsigned long dwDataType,bufferlength;
	plResult=lResult=RegOpenKeyEx(HKEY_CURRENT_USER,AppReg,0,KEY_QUERY_VALUE,&hKey);
	if(lResult != ERROR_SUCCESS)
		plResult=lResult=RegOpenKeyEx(HKEY_CURRENT_USER,AppReg " v0.01",0,KEY_QUERY_VALUE,&hKey);
	if(lResult != ERROR_SUCCESS)
		plResult=lResult=RegOpenKeyEx(HKEY_CURRENT_USER,AppReg " v0.02",0,KEY_QUERY_VALUE,&hKey);
	
	if (lResult == ERROR_SUCCESS) {
		bufferlength = sizeof(titlefmt);
		RegQueryValueEx(hKey,"Title Format",0,&dwDataType,titlefmt,&bufferlength);
		bufferlength = sizeof(soundEcho);
		RegQueryValueEx(hKey,"Echo",0,&dwDataType,(BYTE*)&soundEcho,&bufferlength);
		bufferlength = sizeof(soundLowPass);
		RegQueryValueEx(hKey,"Low-pass Filter",0,&dwDataType,(BYTE*)&soundLowPass,&bufferlength);
		bufferlength = sizeof(soundReverse);
		RegQueryValueEx(hKey,"Reverse Stereo",0,&dwDataType,(BYTE*)&soundReverse,&bufferlength);
		bufferlength = sizeof(IgnoreTrackLength);
		RegQueryValueEx(hKey,"Ignore Track Limits",0,&dwDataType,(BYTE*)&IgnoreTrackLength,&bufferlength);
		//bufferlength = sizeof(soundQuality);
		//RegQueryValueEx(hKey,"Sample Rate",0,&dwDataType,(BYTE*)&soundQuality,&bufferlength);
		
		//bufferlength = sizeof(FastSeek);
		//RegQueryValueEx(hKey,"Fast Seek",0,&dwDataType,(BYTE*)&FastSeek,&bufferlength);
		//bufferlength = sizeof(AudioHLE);
		//RegQueryValueEx(hKey,"Audio HLE",0,&dwDataType,(BYTE*)&AudioHLE,&bufferlength);
		
		bufferlength = sizeof(PepperisaGoodBoy);
		RegQueryValueEx(hKey,"Pepper is a Good Boy",0,&dwDataType,(BYTE*)&PepperisaGoodBoy,&bufferlength);
		
		bufferlength = sizeof(DefaultLength);
		RegQueryValueEx(hKey,"Default Length",0,&dwDataType,(BYTE*)&DefaultLength,&bufferlength);
		
		bufferlength = sizeof(deflen);
		RegQueryValueEx(hKey,"Default Length Value",0,&dwDataType,(BYTE*)&deflen,&bufferlength);

		bufferlength = sizeof(deffade);
		RegQueryValueEx(hKey,"Default Fade Value",0,&dwDataType,(BYTE*)&deffade,&bufferlength);
		
		bufferlength = sizeof(DetectSilence);
		RegQueryValueEx(hKey,"Detect Silence",0,&dwDataType,(BYTE*)&DetectSilence,&bufferlength);

		bufferlength = sizeof(silencelength);
		RegQueryValueEx(hKey,"Detect Silence Value",0,&dwDataType,(BYTE*)&silencelength,&bufferlength);

		//bufferlength = sizeof(UseRecompiler);
		//RegQueryValueEx(hKey,"Use Recompiler",0,&dwDataType,(BYTE*)&UseRecompiler,&bufferlength);

		//bufferlength = sizeof(CPUPriority);
		//RegQueryValueEx(hKey,"CPU Thread Priority",0,&dwDataType,(BYTE*)&CPUPriority,&bufferlength);
		if(plResult != ERROR_SUCCESS)
			WriteSettings();

		RegCloseKey(hKey);
	} else {
		WriteSettings();
	}
}

