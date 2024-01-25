#ifndef DIALOG_H
#define DIALOG_H

#include "interpreter.h"

namespace fallout {

typedef void DialogFunc1(int win);
typedef void DialogFunc2(int win);

typedef struct STRUCT_OPTION {
    char* Str1;
    union {
        int proc;
        char* nodeName;
    };
    int kind;
    int y;
    int optionHeight;
    int Cnt;
    short font;
    short flags;
} STRUCT_OPTION;

typedef struct STRUCT_REPLY {
    char* title;
    char* nodeName;
    char* text;
    STRUCT_OPTION* option;
    int top;
    int option_count;
    int font; // probably font number
    short flags;
} STRUCT_REPLY;

typedef struct STRUCT_DIALOG {
    Program* program;
    STRUCT_REPLY* reply;
    int reply_count;
    int cnt2;
    int cnt3;
    int field_14;
    int field_18;
} STRUCT_DIALOG;

extern const float flt_501623;
extern const float flt_501627;

extern int gDepth;
extern int _topDialogLine;
extern int _topDialogReply;
extern DialogFunc1* _replyWinDrawCallback;
extern DialogFunc2* _optionsWinDrawCallback;
extern int gSayWinHorizontal;
extern int gSayWinVertical;
extern int gDialogOptionSpacing;
extern int _replyRGBset;
extern int _optionRGBset;
extern int _exitDialog;
extern int _inDialog;
extern int _mediaFlag;

extern STRUCT_DIALOG _dialog[4];
extern short gSayOptFlags;
extern int gOptionW;
extern int gOptionH;
extern int gOptionX;
extern int gOptionY;
extern char* gOptionFile;
extern int gReplyW;
extern int gReplyH;
extern int gReplyX;
extern int gReplyY;
extern char* gReplyFile;
extern int _replyPlaying;
extern int _replyWin;
extern int gDialogReplyColorG;
extern int gDialogReplyColorB;
extern int gDialogOptionColorG;
extern int gDialogReplyColorR;
extern int gDialogOptionColorB;
extern int gDialogOptionColorR;
extern int _downButton;
extern int _downButton2;
extern int _downButton7;
extern char* _downButton3;
extern char* _downButton4;
extern char* _downButton5;
extern char* _downButton6;
extern char* gDialogReplyTitle;
extern int _upButton;
extern int _upButton2;
extern int _upButton7;
extern char* _upButton3;
extern char* _upButton4;
extern char* _upButton5;
extern char* _upButton6;

STRUCT_REPLY* _getReply();
void _replyAddOption(const char* a1, const char* a2, int a3);
void _replyAddOptionProc(const char* a1, int a2, int a3);
void _optionFree(STRUCT_OPTION* a1);
void _replyFree();
int _endDialog();
int _abortReply(int a1);
void _endReply();
int _dialogStart(Program* a1);
int _dialogRestart();
int _dialogGotoReply(const char* a1);
int dialogSetReplyTitle(const char* a1);
int _dialogReply(const char* a1, const char* a2);
int _dialogOption(const char* a1, const char* a2);
int _dialogOptionProc(const char* a1, int a2);
int _dialogMsg(char* a1, char* a2, int timeout);
int _dialogEnd(int a1);
int _dialogGetExitPoint();
int _dialogQuit();
int dialogSetOptionWindow(int x, int y, int w, int h, char* file);
int dialogSetReplyWindow(int x, int y, int w, int h, char* file);
int dialogSetBorder(int a1, int a2);
int _dialogSetScrollUp(int a1, int a2, char* a3, char* a4, char* a5, char* a6, int a7);
int _dialogSetScrollDown(int a1, int a2, char* a3, char* a4, char* a5, char* a6, int a7);
int dialogSetOptionSpacing(int value);
int dialogSetOptionColor(float a1, float a2, float a3);
int dialogSetReplyColor(float a1, float a2, float a3);
int _dialogSetOptionFlags(int flags);
void dialogInit();
void _dialogClose();
int _dialogGetDialogDepth();
void _dialogRegisterWinDrawCallbacks(DialogFunc1* a1, DialogFunc2* a2);
int _dialogToggleMediaFlag(int a1);
int _dialogGetMediaFlag();

} // namespace fallout

#endif /* DIALOG_H */
