#include "dialog.h"

#include <string.h>

#include "memory_manager.h"
#include "mouse.h"
#include "movie.h"
#include "platform_compat.h"
#include "svga.h"
#include "text_font.h"
#include "window_manager.h"
#include "datafile.h"
#include "draw.h"
#include "scripts.h"
#include "debug.h"
#include "kb.h"
#include "game_mouse.h"
#include "input.h"
#include "word_wrap.h"

namespace fallout {

// 0x5184B4
int gDepth = -1;

// 0x5184B8
int _topDialogLine = 0;

// 0x5184BC
int _topDialogReply = 0;

// 0x5184E4
DialogFunc1* _replyWinDrawCallback = nullptr;

// 0x5184E8
DialogFunc2* _optionsWinDrawCallback = nullptr;

// 0x5184EC
int gSayWinHorizontal = 7;

// 0x5184F0
int gSayWinVertical = 7;

// 0x5184F4
int gDialogOptionSpacing = 5;

// 0x5184F8
int _replyRGBset = 0;

// 0x5184FC
int _optionRGBset = 0;

// 0x518500
int _exitDialog = 0;

// 0x518504
int _inDialog = 0;

// 0x518508
int _mediaFlag = 2;

// 0x56DAE0
STRUCT_DIALOG _dialog[4];

// Reply flags.
//
// 0x56DB60
short gSayOptFlags;

// 0x56DB64
int gOptionW;

// 0x56DB68
int gOptionH;

// 0x56DB6C
int gOptionX;

// 0x56DB70
int gOptionY;

// 0x56DB74
char* gOptionFile;

// 0x56DB7C
int gReplyW = 0;

// 0x56DB80
int gReplyH;

// 0x56DB84
int gReplyX;

// 0x56DB88
int gReplyY;

// 0x56DB8C
char* gReplyFile;

// 0x56DB90
int _replyPlaying;

// 0x56DB94
int _replyWin = -1;

// 0x56DB98
int gDialogReplyColorG;

// 0x56DB9C
int gDialogReplyColorB;

// 0x56DBA4
int gDialogOptionColorG;

// 0x56DBA8
int gDialogReplyColorR;

// 0x56DBAC
int gDialogOptionColorB;

// 0x56DBB0
int gDialogOptionColorR;

// 0x56DBB4
int _downButton;

// 0x56DBB8
int _downButton2;

// 0x56DBBC
int _downButton7;

// 0x56DBC0
char* _downButton3;

// 0x56DBC4
char* _downButton4;

// 0x56DBC8
char* _downButton5;

// 0x56DBCC
char* _downButton6;

// 0x56DBD0
char* gDialogReplyTitle;

// 0x56DBD4
int _upButton;

// 0x56DBD8
int _upButton2;

// 0x56DBDC
int _upButton7;

// 0x56DBE0
char* _upButton3;

// 0x56DBE4
char* _upButton4;

// 0x56DBE8
char* _upButton5;

// 0x56DBEC
char* _upButton6;

int gSayUnk124;
int gSayUnk122;
int gSayUnk111;

// 0x42F434
STRUCT_REPLY* _getReply()
{
    STRUCT_REPLY* v0;
    STRUCT_OPTION* v1;

    v0 = &(_dialog[gDepth].reply[_dialog[gDepth].cnt2]);
    if (v0->option == nullptr) {
        v0->option_count = 1;
        v1 = (STRUCT_OPTION*)internal_malloc_safe(sizeof(STRUCT_OPTION), __FILE__, __LINE__); // "..\\int\\DIALOG.C", 789
    } else {
        v0->option_count++;
        v1 = (STRUCT_OPTION*)internal_realloc_safe(v0->option, sizeof(STRUCT_OPTION) * v0->option_count, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 793
    }
    v0->option = v1;

    return v0;
}

// 0x42F4C0
void _replyAddOption(const char* label, const char* nodeName, int a3)
{
    STRUCT_REPLY* v18;
    int v17;
    char* v14;
    char* v15;

    v18 = _getReply();
    v17 = v18->option_count - 1;
    v18->option[v17].kind = 2;

    if (label != nullptr) {
        v14 = (char*)internal_malloc_safe(strlen(label) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 805
        strcpy(v14, label);
        v18->option[v17].Str1 = v14;
    } else {
        v18->option[v17].Str1 = nullptr;
    }

    if (nodeName != nullptr) {
        v15 = (char*)internal_malloc_safe(strlen(nodeName) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 810
        strcpy(v15, nodeName);
        v18->option[v17].nodeName = v15;
    } else {
        v18->option[v17].nodeName = nullptr;
    }

    v18->option[v17].font = windowGetFont();
    v18->option[v17].flags = gSayOptFlags;
    v18->option[v17].Cnt = a3;
}

// 0x42F624
void _replyAddOptionProc(const char* msg, int proc, int a3)
{
    STRUCT_REPLY* reply;
    int i;

    reply = _getReply();
    i = reply->option_count - 1;

    reply->option[i].kind = 1;

    if (msg != nullptr) {
        char* tmp = (char*)internal_malloc_safe(strlen(msg) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 830
        strcpy(tmp, msg);
        reply->option[i].Str1 = tmp;
    } else {
        reply->option[i].Str1 = nullptr;
    }

    reply->option[i].proc = proc;

    reply->option[i].font = windowGetFont();
    reply->option[i].flags = gSayOptFlags;
    reply->option[i].Cnt = a3;
}

// 0x42F714
void _optionFree(STRUCT_OPTION* a1)
{
    if (a1->Str1 != nullptr) {
        internal_free_safe(a1->Str1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 844
    }

    if (a1->kind == 2) {
        if (a1->nodeName != nullptr) {
            internal_free_safe(a1->nodeName, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 846
        }
    }
}

// 0x42F754
void _replyFree()
{
    int i;
    int j;
    STRUCT_DIALOG* ptr;
    STRUCT_REPLY* v6;

    ptr = &(_dialog[gDepth]);
    for (i = 0; i < ptr->reply_count; i++) {
        v6 = &(_dialog[gDepth].reply[i]);

        if (v6->option != nullptr) {
            for (j = 0; j < v6->option_count; j++) {
                _optionFree(&(v6->option[j]));
            }

            internal_free_safe(v6->option, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 857
        }

        if (v6->text != nullptr) {
            internal_free_safe(v6->text, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 860
        }

        if (v6->nodeName != nullptr) {
            internal_free_safe(v6->nodeName, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 862
        }

        if (v6->title != nullptr) {
            internal_free_safe(v6->title, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 864
        }
    }

    if (ptr->reply != nullptr) {
        internal_free_safe(ptr->reply, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 867
    }
}

int GetWrapCount(char* string, int width)
{
    if (string == nullptr) {
        return 0;
    }

    short beginnings[WORD_WRAP_MAX_COUNT] = {
        -1,
    };
    short count = -1;
    if (wordWrap(string, width, beginnings, &count) != 0) {
        return 0;
    }
    
    return count - 1;
}

// 0x42FB94
int _endDialog()
{
    if (gDepth == -1) {
        return -1;
    }

    _topDialogReply = _dialog[gDepth].cnt3;
    _replyFree();

    if (gDialogReplyTitle != nullptr) {
        internal_free_safe(gDialogReplyTitle, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 986
        gDialogReplyTitle = nullptr;
    }

    --gDepth;

    return 0;
}

// 0x430104
int _abortReply(int a1)
{
    int result;
    int y;
    int x;

    if (_replyPlaying == 2) {
        return _moviePlaying() == 0;
    } else if (_replyPlaying == 3) {
        return 1;
    }

    result = 1;
    if (a1) {
        if (_replyWin != -1) {
            if (!(mouseGetEvent() & 0x10)) {
                result = 0;
            } else {
                mouseGetPosition(&x, &y);

                if (windowGetAtPoint(x, y) != _replyWin) {
                    result = 0;
                }
            }
        }
    }
    return result;
}

// 0x430180
void _endReply()
{
    if (_replyPlaying != 2) {
        if (_replyPlaying == 1) {
            if (!(_mediaFlag & 2) && _replyWin != -1) {
                windowDestroy(_replyWin);
                _replyWin = -1;
            }
        } else if (_replyPlaying != 3 && _replyWin != -1) {
            windowDestroy(_replyWin);
            _replyWin = -1;
        }
    }
}

// 0x430D40
int _dialogStart(Program* a1)
{
    STRUCT_DIALOG* ptr;

    if (gDepth == 3) {
        return 1;
    }

    gDepth++;

    ptr = &(_dialog[gDepth]);
    ptr->program = a1;
    ptr->reply = 0;
    ptr->reply_count = 0;
    ptr->cnt2 = -1;
    ptr->cnt3 = -1;
    ptr->field_14 = 1;
    ptr->field_18 = 1;

    return 0;
}

// 0x430DB8
int _dialogRestart()
{
    if (gDepth == -1) {
        return 1;
    }

    _dialog[gDepth].cnt3 = 0;

    return 0;
}

// 0x430DE4
int _dialogGotoReply(const char* nodeName)
{
    STRUCT_DIALOG* ptr;
    STRUCT_REPLY* v5;
    int i;

    if (gDepth == -1) {
        return 1;
    }

    if (nodeName != nullptr) {
        ptr = &(_dialog[gDepth]);
        for (i = 0; i < ptr->reply_count; i++) {
            v5 = &(ptr->reply[i]);
            if (v5->nodeName != nullptr && compat_stricmp(v5->nodeName, nodeName) == 0) {
                ptr->cnt3 = i;
                return 0;
            }
        }

        return 1;
    }

    _dialog[gDepth].cnt3 = 0;

    return 0;
}

// 0x430E84
int dialogSetReplyTitle(const char* title)
{
    if (gDialogReplyTitle != nullptr) {
        internal_free_safe(gDialogReplyTitle, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2561
    }

    if (title != nullptr) {
        gDialogReplyTitle = (char*)internal_malloc_safe(strlen(title) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2564
        strcpy(gDialogReplyTitle, title);
    } else {
        gDialogReplyTitle = nullptr;
    }

    return 0;
}

// 0x430EFC
int _dialogReply(const char* nodeName, const char* text)
{
    char* s;

    int replycnt = _dialog[gDepth].reply_count;
    _dialog[gDepth].cnt2 = replycnt;
    if (_dialog[gDepth].reply) {
        _dialog[gDepth].reply_count++;
        _dialog[gDepth].reply = (STRUCT_REPLY *)internal_realloc_safe(_dialog[gDepth].reply, sizeof(STRUCT_REPLY) * (replycnt + 1), __FILE__, __LINE__);
    } else {
        _dialog[gDepth].reply = (STRUCT_REPLY *)internal_malloc_safe(sizeof(STRUCT_REPLY), __FILE__, __LINE__);
        _dialog[gDepth].reply_count = 1;
    }
    if (nodeName) {
        s = (char *)internal_malloc_safe(strlen(nodeName) + 1,__FILE__, __LINE__);
        strcpy(s, nodeName);
        _dialog[gDepth].reply[replycnt].nodeName = s;
    } else {
        _dialog[gDepth].reply[replycnt].nodeName = nullptr;
        _dialog[gDepth].cnt3 = replycnt;
    }
    _dialog[gDepth].reply[replycnt].text = nullptr;
    if (text) {
        s = (char *)internal_malloc_safe(strlen(text) + 1, __FILE__, __LINE__);
        strcpy(s, text);
        _dialog[gDepth].reply[replycnt].text = s;
    }
    _dialog[gDepth].reply[replycnt].option = nullptr;
    _dialog[gDepth].reply[replycnt].option_count = 0;
    _dialog[gDepth].reply[replycnt].top = 0;
    _dialog[gDepth].reply[replycnt].font = windowGetFont();
    _dialog[gDepth].reply[replycnt].flags = gSayUnk111;
    _dialog[gDepth].reply[replycnt].title = nullptr;
    if (gDialogReplyTitle != nullptr) {
        s = (char *)internal_malloc_safe(strlen(gDialogReplyTitle) + 1, __FILE__, __LINE__);
        strcpy(s, gDialogReplyTitle);
        _dialog[gDepth].reply[replycnt].title = s;
    }


    return 0;
}

// 0x430F04
int _dialogOption(const char* label, const char* nodeName)
{
    if (_dialog[gDepth].cnt2 == -1) {
        return 0;
    }

    _replyAddOption(label, nodeName, 0);

    return 0;
}

// 0x430F38
int _dialogOptionProc(const char* msg, int proc)
{
    if (_dialog[gDepth].cnt2 == -1) {
        return 1;
    }

    _replyAddOptionProc(msg, proc, 0);

    return 0;
}

int drawSayWindow(int x, int y, int w, int h, int Color, char* fname)
{
    debugPrint("drawSayWindow %s", fname);
    int win, pW, pH;
    unsigned char* data;

    win = windowCreate(x, y, w, h, Color, 0x25);
    if (!fname) {
        windowFill(win, 0, 0, w, h, Color);
        if (!_replyRGBset) windowDrawBorder(win);
        return win;
    }

    data = datafileRead(fname, &pW, &pH);
    if (data != nullptr) {
        blitBufferToBufferStretch(data, pW, pH, pW, windowGetBuffer(win), windowGetWidth(win), windowGetHeight(win), windowGetWidth(win));
        internal_free_safe(data, __FILE__, __LINE__); // "..\\int\\WINDOW.C", 1294
        return win;
    }
    windowFill(win, 0, 0, w, h, Color);
    if (!_replyRGBset) windowDrawBorder(win);
    return win;
}

int drawReply(char* string, char* text, int a3, int a4, int textAlignment)
{
    debugPrint("drawReply strig=%s text=%s", string, text);
    int color, win, fontcolor, v25;
    int x, y, w, h;
    int screenWidth = screenGetWidth();
    int screenHeight = screenGetHeight();

    int stringH = 0;
    h = 0;
    fontcolor = windowGetTextColor() + windowGetTextFlags();

    if (text == nullptr && string == nullptr)
        return 0;
    w = (gReplyW) ? gReplyW : (300 * screenWidth / 640);

    if (string != nullptr)
        stringH = GetWrapCount(string, w - 2 * gSayWinHorizontal) * fontGetLineHeight();

    if (gReplyH) {
        h = gReplyH;
    } else {
        h = 2 * gSayWinVertical;
        h += stringH;
        if (text != nullptr) {
            h += GetWrapCount(text, w - 2 * gSayWinHorizontal) * fontGetLineHeight();
        }
    }

    if (h == 0)
        return 0;

    if (gReplyY) {
        y = gReplyY;
    } else if (a3 == -1) {
        y = screenHeight / 2 - h / 2;
        if (y < 0)
            y = 0;
    } else {
        y = screenHeight * a3 / 480;
    }
    if (gReplyX) {
        x = gReplyX;
    } else {
        x = screenWidth / 2 - w / 2;
        if (x < 0)
            x = 0;
    }

    if (_replyRGBset)
        color = windowGetTextColor(gDialogOptionColorR, gDialogOptionColorG, gDialogOptionColorB);
    else
        color = windowGetTextColor(8, 8, 8);

    win = drawSayWindow(x, y, w, h, color, gReplyFile);

    if (_optionsWinDrawCallback)
        _optionsWinDrawCallback(win);

    if (string != nullptr)
        _windowWrapLine(win, string, w - 2 * gSayWinHorizontal, h, gSayWinHorizontal, gSayWinVertical, fontcolor, TEXT_ALIGNMENT_CENTER);

    if (text != nullptr)
        _windowWrapLine(win, text, w - 2 * gSayWinHorizontal, h - stringH, gSayWinHorizontal, gSayWinVertical + stringH, fontcolor, textAlignment);

    if (a4) {
        windowShow(win);
        windowRefresh(win);
    }

    return win;
}

// 0x430FD4
int _dialogMsg(char* a1, char* a2, int timeout)
{
    //WinGetUnk01(1);
    int SysTime = getTicks();
    int WinId = drawReply(a1, a2, -1, 1, gSayUnk111);
    if (WinId) {
        while (1) {
            sharedFpsLimiter.mark();

            inputGetInput2();
            if (a2 != nullptr) {
                if ((getTicks() - SysTime) / 1000 > timeout) {
                    renderPresent();
                    sharedFpsLimiter.throttle();
                    break;
                }
            }
            if ((mouseGetEvent() & 0x10) != 0) {
                int x, y;
                mouseGetPosition(&x, &y);
                if (windowGetAtPoint(x, y) == WinId) {

                    renderPresent();
                    sharedFpsLimiter.throttle();
                    break;
                }
            }

            renderPresent();
            sharedFpsLimiter.throttle();
        }
        windowDestroy(WinId);
    }
    //WinGetUnk01(0);
    return 0;
}

void drawReplyIndex(int index)
{
    debugPrint("drawReplyIndex %d", index);
    STRUCT_REPLY* reply;

    _replyWin = -1;
    _replyPlaying = 0;
    if (!gSayUnk124 && !gSayUnk122) {
        reply = &_dialog[gDepth].reply[index];
        _replyWin = drawReply(reply->title, reply->text, 50, 0, reply->flags);
        windowShow(_replyWin);
        windowRefresh(_replyWin);
        gameMouseRefresh();
        mouseShowCursor();
    }
}

int DialogUnk21(int a1)
{
    debugPrint("DialogUnk21");
    int v2, right;

    if (_replyPlaying == 2)
        return _moviePlaying() == 0;
    if (_replyPlaying == 3)
        return 1;
    if (!a1)
        return 1;
    if (_replyWin == -1)
        return 1;

    if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
        mouseGetPosition(&right, &v2);
        if (windowGetAtPoint(right, v2) == _replyWin)
            return 1;
    }
    return 0;
}

void DialogPrint(int WinId, char* Name, int FontId, int w, int h, int x, int y, int color, int a9, int a9a)
{
    debugPrint("DialogPrint %s", Name);
    int SaveFont;
    Rect Area;

    SaveFont = windowGetFont();
    windowSetFont(FontId);
    _windowWrapLineWithSpacing(WinId, Name, w, h, x, y, color, a9, a9a);
    Area.left = x;
    Area.top = y;
    Area.right = x + w;
    Area.bottom = y + h;
    windowRefreshRect(WinId, &Area);
    windowSetFont(SaveFont);
}

int drawOptionIndex(int Win, int top, int WinW, int WinH, int count, int Flags)
{
    debugPrint("drawOptionIndex %d-%d", top, count);
    char **Lines;
    int Cnt3, hy, ArtH, ArtW, i;

    Cnt3 = _dialog[gDepth].cnt3;
    if (gOptionFile) {
        unsigned char* file = datafileRead(gOptionFile, &ArtW, &ArtH);
        if (file != nullptr) {
            blitBufferToBufferStretch(file, ArtW, ArtH, ArtW, windowGetBuffer(Win), windowGetWidth(Win), windowGetHeight(Win), windowGetWidth(Win));
            internal_free_safe(file, __FILE__, __LINE__); // "..\\int\\WINDOW.C", 1294
        } else {
            if (_optionRGBset) {
                windowFill(Win, 0, 0, WinW, WinH, windowGetTextColor(gDialogOptionColorR, gDialogOptionColorG, gDialogOptionColorB));
            } else {
                windowFill(Win, 0, 0, WinW, WinH, windowGetTextColor(8, 8, 8));
                windowDrawBorder(Win);
            }
        }
    } else {
        if (_optionRGBset) {
            windowFill(Win, 0, 0, WinW, WinH, windowGetTextColor(gDialogOptionColorR, gDialogOptionColorG, gDialogOptionColorB));
        } else {
            windowFill(Win, 0, 0, WinW, WinH, windowGetTextColor(8, 8, 8));
            windowDrawBorder(Win);
        }
    }

    if (_replyWinDrawCallback)
        _replyWinDrawCallback(Win);
    
    int Font = windowGetFont();
    int DisplLines = 0;
    hy = 0;

    for (i = top; i < count; i++) {
        windowSetFont(_dialog[gDepth].reply[Cnt3].option[i].font);

        int count = _windowWrapLineWithSpacing(Win,
            _dialog[gDepth].reply[Cnt3].option[i].Str1,
            WinW - 2 * gSayWinHorizontal,
            WinH - 2 * gSayWinVertical - hy,
            gSayWinHorizontal,
            gSayWinVertical + hy,
            Flags, _dialog[gDepth].reply[Cnt3].option[i].flags, 0);

        int Height = fontGetLineHeight() * count;
        DisplLines++;
        _dialog[gDepth].reply[Cnt3].option[i].y = gSayWinVertical + hy;
        _dialog[gDepth].reply[Cnt3].option[i].optionHeight = Height;
        hy += Height + gDialogOptionSpacing;
    }

    windowSetFont (Font);
    return DisplLines;
}

int drawOption(int optionIndex)
{
    debugPrint("drawOption %d", optionIndex);
    STRUCT_OPTION* p;
    int Cnt3, Font, i,  v34, sel, v38, v55, MseY, MseX, v60, x, y, selected, h;
    int ColorA, w, v70, win;

    selected = v70 =  -1;
    Cnt3 = _dialog[gDepth].cnt3;
    int option_count = _dialog[gDepth].reply[Cnt3].option_count;
    _topDialogLine = (option_count) ? optionIndex : -1;

    if (option_count <= 1) {
        if (gSayUnk122) 
            return _topDialogLine;
        debugPrint("reply has only one option, waiting for dialog to end\n");

        do {
            sharedFpsLimiter.mark();
            inputGetInput2();
            renderPresent();
            sharedFpsLimiter.throttle();
        }
        while (!_exitDialog && !DialogUnk21(1));

        debugPrint("Ended\n");
            
        _endReply();
        _exitDialog = 0;
        if (option_count)
            _dialog[gDepth].reply[Cnt3].top = _topDialogLine;
        return _topDialogLine;
    }

    inputGetInput2();
    debugPrint("waiting for reply to end\n");
    while (!_exitDialog) {
        if (DialogUnk21(option_count <= 1)) 
            break;

        sharedFpsLimiter.mark();
        inputGetInput2();
        renderPresent();
        sharedFpsLimiter.throttle();
    }
    debugPrint("reply ended\n");

    w = (gOptionW) ? gOptionW : (300 * screenGetWidth() / 640);

    Font = windowGetFont();
    int tmp = gSayWinVertical;
    for (i = 0; i < option_count; i++) {
        windowSetFont(_dialog[gDepth].reply[Cnt3].option[i].font);
        tmp += gDialogOptionSpacing + GetWrapCount(_dialog[gDepth].reply[Cnt3].option[i].Str1, w - 2 * gSayWinHorizontal) * fontGetLineHeight();
    }
    windowSetFont(Font);
    h = (gOptionH) ? gOptionH : (gSayWinVertical - gDialogOptionSpacing + tmp);

    x = gOptionX;
    if (!gOptionX) {
        x = (screenGetWidth() - w) / 2;
    }

    y = (gOptionY) ? gOptionY : (320 * screenGetHeight() / 480);
    if ((y + h) > screenGetHeight())
        y = screenGetHeight() - h;
    
    ColorA = windowGetTextFlags() + windowGetTextColor();
    v60 = windowGetTextFlags() + windowGetHighlightColor();
    if (_optionRGBset)
        win = drawSayWindow(x, y, w, h, windowGetTextColor(gDialogOptionColorR,gDialogOptionColorG,gDialogOptionColorB), gOptionFile);
    else
        win = drawSayWindow(x, y, w, h, windowGetTextColor(8,8,8), gOptionFile);
    
    buttonCreate(win, 0, 0, w, h, -1, -1, -1, 256, 0, 0, 0, 0);
    v55 = drawOptionIndex(win, _topDialogLine, w, h, option_count, ColorA);
    windowShow(win);
    windowRefresh(win);
    gameMouseRefresh();
    mouseShowCursor();
    
    v34 = Cnt3;
    while (selected == -1) {
        sharedFpsLimiter.mark();

        sel = inputGetInput2();
        if (_exitDialog || sel == KEY_ESCAPE) {
            selected = -1;
            break;
        }

        mouseGetPosition(&MseX, &MseY);
        gameMouseRefresh();
        
        int PointedWidgetId = windowGetAtPoint(MseX, MseY);
        if (PointedWidgetId != win && v70 != -1) {
            p = &_dialog[gDepth].reply[v34].option[v70];
            DialogPrint(win, p->Str1, p->font, w - 2 * gSayWinHorizontal, p->optionHeight, gSayWinHorizontal, p->y, ColorA, p->flags, 1);
            v70 = -1;
        }
        MseX -= x;
        MseY -= y;
        if (PointedWidgetId == win) {
            v38 = gSayWinVertical;
            for (i = _topDialogLine; i < option_count; i++) {
                p = &_dialog[gDepth].reply[v34].option[i];
                if ((p->optionHeight + v38) > (h - gSayWinVertical)) {
                    i = option_count;
                    break;
                }
                v38 += gDialogOptionSpacing + p->optionHeight;
                if (p->y <= MseY && p->y + p->optionHeight >= MseY)
                    break;
            }
            if (i == option_count) {
                if (v70 != -1) {
                    p = &_dialog[gDepth].reply[v34].option[v70];
                    v70 = -1;
                    DialogPrint(win, p->Str1, p->font, w - 2 * gSayWinHorizontal, p->optionHeight, gSayWinHorizontal, p->y, ColorA, p->flags, 1);
                }
                renderPresent();
                sharedFpsLimiter.throttle();
                continue;
            }
            if (i != v70) {
                if (v70 != -1) {
                    p = &_dialog[gDepth].reply[v34].option[v70];
                    DialogPrint(win, p->Str1, p->font, w - 2 * gSayWinHorizontal, p->optionHeight, gSayWinHorizontal, p->y, ColorA, p->flags, 1);
                }
                v70 = i;
                p = &_dialog[gDepth].reply[v34].option[i];
                DialogPrint(win, p->Str1, p->font, w - 2 * gSayWinHorizontal, p->optionHeight, gSayWinHorizontal, p->y, v60, p->flags, 1);
            }
        }

        switch (sel) {
        case 256:
            selected = i;
            break;
        case 512:
            if (--_topDialogLine < 0)
                _topDialogLine = 0;
            v55 = drawOptionIndex(win, _topDialogLine, w, h, option_count, ColorA);
            v70 = -1;
            windowShow(win);
            windowRefresh(win);
            break;
        case 1024:
            if ((_topDialogLine + 1 + v55) > option_count)
                break;
            _topDialogLine++;
            if ((_topDialogLine + 1) >= option_count)
                _topDialogLine = option_count;
            v55 = drawOptionIndex(win, _topDialogLine, w, h, option_count, ColorA);
            v70 = -1;
            windowShow(win);
            windowRefresh(win);
            break;
        }
        
        renderPresent();
        sharedFpsLimiter.throttle();
    }

    _endReply();
    windowDestroy(win);
    _exitDialog = 0;
    _dialog[gDepth].reply[Cnt3].top = _topDialogLine;
    
    return selected;
}

int drawDialog()
{
    debugPrint("drawDialog");
    int Cnt3, n, Font;

    Cnt3 = _dialog[gDepth].cnt3;
    gSayUnk124 = 0;
    gSayUnk122 = 0;
    if (Cnt3 < 0) 
        return -1;
    Font = windowGetFont();
    windowSetFont(_dialog[gDepth].reply[Cnt3].font);
    drawReplyIndex(_dialog[gDepth].cnt3);
    n = drawOption(_dialog[gDepth].reply[Cnt3].top);
    windowSetFont(Font);
    return n;
}

int _executeOption(int a1)
{
    debugPrint("_executeOption");
    int Flags, i, v12;
    char* Str2;
    STRUCT_REPLY* v7;

    Flags = _dialog[gDepth].reply[_dialog[gDepth].cnt3].option[a1].kind;
    //if (gSayUnk123) gSayUnk123(a1, a1);
    if (!Flags) return 1;
    if (Flags > 1) {
        if (Flags == 2) {
            if ((Str2 = _dialog[gDepth].reply[_dialog[gDepth].cnt3].option[a1].nodeName)) {
                for (i = 0; i < _dialog[gDepth].reply_count; i++) {
                    v7 = &_dialog[gDepth].reply[i];
                    if (v7->nodeName && !strcmp(v7->nodeName, Str2)) {
                        _dialog[gDepth].cnt3 = i;
                        return 0;
                    }
                }
            }
        }
        return 1;
    }
    _dialog[gDepth].field_18++;
    v12 = _dialog[gDepth].cnt3;
    _executeProcedure(_dialog[gDepth].program, _dialog[gDepth].reply[_dialog[gDepth].cnt3].option[a1].proc);
    _dialog[gDepth].field_18--;
    return v12 == _dialog[gDepth].cnt3;
}

int doDialogEnd(int a1)
{
    int v1 = -1;
    int procid = -1;

    _inDialog++;
    while (gDepth != -1) {
        procid = drawDialog();
        if (procid == v1) 
            break;
        v1 = _executeOption(procid);
        if (v1) 
            break;
    }
    _inDialog--;
    return v1;
}

// 0x431088
int _dialogEnd(int top)
{
    int v5, Cnt, v8 = 0;

    bool IsCursorHidden = cursorIsHidden();
    //MouseMgrRedraw();

    if (_dialog[gDepth].field_18 != 1) {
        //TextBoxUnk03("Error, can't call sayEnd inside of option target procedure");
        return -1;
    }
    if (top != -1) {
        v5 = (top >> 24) & 0xff;
        if (v5 >= _dialog[gDepth].reply_count) {
            _endDialog();
            return -1;
        }
        if ((Cnt = _dialog[gDepth].reply[v5].option_count)) {
            if (top >= Cnt) {
                _endDialog();
                return -1;
            }
        }
        _dialog[gDepth].cnt3 = v5;
        _dialog[gDepth].reply[v5].top = top;
    }
    
    doDialogEnd(7 * v5);
    
    if (_endDialog()) 
        v8 = -1;
    
    if (IsCursorHidden) 
        mouseShowCursor();
    return v8;
}

// 0x431184
int _dialogGetExitPoint()
{
    return _topDialogLine + (_topDialogReply << 16);
}

// 0x431198
int _dialogQuit()
{
    if (_inDialog) {
        _exitDialog = 1;
    } else {
        _endDialog();
    }

    return 0;
}

// 0x4311B8
int dialogSetOptionWindow(int x, int y, int w, int h, char* file)
{
    gOptionX = x;
    gOptionY = y;
    gOptionW = w;
    gOptionH = h;
    gOptionFile = file;

    return 0;
}

// 0x4311E0
int dialogSetReplyWindow(int x, int y, int w, int h, char* file)
{
    gReplyX = x;
    gReplyY = y;
    gReplyW = w;
    gReplyH = h;
    gReplyFile = file;

    return 0;
}

// 0x431208
int dialogSetBorder(int a1, int a2)
{
    gSayWinHorizontal = a1;
    gSayWinVertical = a2;

    return 0;
}

// 0x431218
int _dialogSetScrollUp(int a1, int a2, char* a3, char* a4, char* a5, char* a6, int a7)
{
    _upButton = a1;
    _upButton2 = a2;

    if (_upButton3 != nullptr) {
        internal_free_safe(_upButton3, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2750
    }
    _upButton3 = a3;

    if (_upButton4 != nullptr) {
        internal_free_safe(_upButton4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2752
    }
    _upButton4 = a4;

    if (_upButton5 != nullptr) {
        internal_free_safe(_upButton5, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2754
    }
    _upButton5 = a5;

    if (_upButton6 != nullptr) {
        internal_free_safe(_upButton6, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2756
    }
    _upButton6 = a6;

    _upButton7 = a7;

    return 0;
}

// 0x4312C0
int _dialogSetScrollDown(int a1, int a2, char* a3, char* a4, char* a5, char* a6, int a7)
{
    _downButton = a1;
    _downButton2 = a2;

    if (_downButton3 != nullptr) {
        internal_free_safe(_downButton3, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2765
    }
    _downButton3 = a3;

    if (_downButton4 != nullptr) {
        internal_free_safe(_downButton4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2767
    }
    _downButton4 = a4;

    if (_downButton5 != nullptr) {
        internal_free_safe(_downButton5, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2769
    }
    _downButton5 = a5;

    if (_downButton6 != nullptr) {
        internal_free_safe(_downButton6, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2771
    }
    _downButton6 = a6;

    _downButton7 = a7;

    return 0;
}

// 0x431368
int dialogSetOptionSpacing(int value)
{
    gDialogOptionSpacing = value;

    return 0;
}

// 0x431370
int dialogSetOptionColor(float a1, float a2, float a3)
{
    gDialogOptionColorR = (int)(a1 * 31.0);
    gDialogOptionColorG = (int)(a2 * 31.0);
    gDialogOptionColorB = (int)(a3 * 31.0);

    _optionRGBset = 1;

    return 0;
}

// 0x4313C8
int dialogSetReplyColor(float a1, float a2, float a3)
{
    gDialogReplyColorR = (int)(a1 * 31.0);
    gDialogReplyColorG = (int)(a2 * 31.0);
    gDialogReplyColorB = (int)(a3 * 31.0);

    _replyRGBset = 1;

    return 0;
}

// 0x431420
int _dialogSetOptionFlags(int flags)
{
    gSayOptFlags = flags & 0xFFFF;

    return 1;
}

// 0x431430
void dialogInit()
{
}

// 0x431434
void _dialogClose()
{
    if (_upButton3) {
        internal_free_safe(_upButton3, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2818
    }

    if (_upButton4) {
        internal_free_safe(_upButton4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2819
    }

    if (_upButton5) {
        internal_free_safe(_upButton5, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2820
    }

    if (_upButton6) {
        internal_free_safe(_upButton6, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2821
    }

    if (_downButton3) {
        internal_free_safe(_downButton3, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2823
    }

    if (_downButton4) {
        internal_free_safe(_downButton4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2824
    }

    if (_downButton5) {
        internal_free_safe(_downButton5, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2825
    }

    if (_downButton6) {
        internal_free_safe(_downButton6, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2826
    }
}

// 0x431518
int _dialogGetDialogDepth()
{
    return gDepth;
}

// 0x431520
void _dialogRegisterWinDrawCallbacks(DialogFunc1* a1, DialogFunc2* a2)
{
    _replyWinDrawCallback = a1;
    _optionsWinDrawCallback = a2;
}

// 0x431530
int _dialogToggleMediaFlag(int a1)
{
    if ((a1 & _mediaFlag) == a1) {
        _mediaFlag &= ~a1;
    } else {
        _mediaFlag |= a1;
    }

    return _mediaFlag;
}

// 0x431554
int _dialogGetMediaFlag()
{
    return _mediaFlag;
}

} // namespace fallout
