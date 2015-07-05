#ifndef __MISC_H__
#define __MISC_H__

#undef s_addr

typedef struct {
	unsigned char id[8];
    unsigned long text;                   
    unsigned long data;                    
    unsigned long pc0;
    unsigned long gp0;                     
    unsigned long t_addr;
    unsigned long t_size;
    unsigned long d_addr;                  
    unsigned long d_size;                  
    unsigned long b_addr;                  
    unsigned long b_size;                  
    unsigned long s_addr;
    unsigned long s_size;
    unsigned long SavedSP;
    unsigned long SavedFP;
    unsigned long SavedGP;
    unsigned long SavedRA;
    unsigned long SavedS0;
} EXE_HEADER;

extern char CdromId[10];
extern char CdromLabel[33];

int LoadCdrom();
int LoadCdromFile(char *filename, EXE_HEADER *head);
int CheckCdrom();
int Load(char *ExePath);

int SaveState(char *file);
int LoadState(char *file);
int SaveStateEmufile(EMUFILE *f);
int LoadStateEmufile(EMUFILE *f);

int CheckState(char *file);

int SaveStateEmbed(char *file);
int LoadStateEmbed(char *file);

int SendPSXjinInfo();
int RecvPSXjinInfo();

extern char *LabelAuthors;
extern char *LabelGreets;

char *ParseLang(char *id);

#endif /* __MISC_H__ */
