#include <stdbool.h>
#include <stdint.h>

#define BOOL bool
#define BYTE uint8_t

extern void DisplayError (char *, ...);

extern BOOL IsTagPresent (BYTE *);
extern BOOL IsValidGSF (BYTE *);
extern void setupSound(void);
extern int GSFRun(char *);
extern void GSFClose(void) ;
extern BOOL EmulationLoop(void);

