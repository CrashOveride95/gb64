
#ifndef _MENU_H
#define _MENU_H

#include <ultra64.h>
#include "sprite.h"

#define INITIAL_TIME_DELAY 500
#define REPEAT_TIME_DELAY 200

struct MenuItem;

typedef void (*MenuItemRender)(struct MenuItem* menuItem, struct MenuItem* highlightedItem);
typedef struct MenuItem* (*MenuItemHandleInput)(struct MenuItem* menuItem, int buttonsDown, int buttonsState);
typedef void (*MenuItemSetActive)(struct MenuItem* menuItem, int isActive);

#define MENU_ITEM_FLAGS_HIDDEN  0x1

struct MenuItem {
    struct MenuItem* toLeft;
    struct MenuItem* toRight;
    struct MenuItem* toUp;
    struct MenuItem* toDown;
    void* data;
    MenuItemRender renderCallback;
    MenuItemHandleInput handleButtonDown;
    MenuItemHandleInput handleButtonUp;
    MenuItemSetActive setActive;
    u32 flags;
};

struct MenuState {
    struct MenuItem* currentMenuItem;
    struct MenuItem* allItems;
    int menuItemCount;
    int lastButtons;
    u32 holdTimer;
};


struct CursorMenuItem;

typedef struct MenuItem* (*InputCursorMenuItem)(struct CursorMenuItem* menuItem, int buttonDown);
typedef void (*RenderCursorMenuItem)(struct CursorMenuItem* menuItem, int x, int y, int selected);

struct CursorMenuItem {
    struct MenuItem* toMenu;
    char* label;
    InputCursorMenuItem input;
    RenderCursorMenuItem render;
    void* data;
    u16 height;
    u16 flags;
};

extern struct SpriteTile gButtonIconTile[];
extern char gButtonIconLayer[];
extern struct SpriteTile gGUIItemTiles[];

enum GUIItemIcon
{
    GUIItemIconRight,
    GUIItemIconLeft,
    GUIItemIconDown,
    GUIItemIconUp,
    GUIItemIconHorz,
    GUIItemIconVert,
    GUIItemIconTopRight,
    GUIItemIconBottomRight,
    GUIItemIconWhite,
    GUIItemIconBorder,
    GUIItemIconBlack,
};

struct CursorMenu {
    struct MenuItem* parentMenu;
    struct CursorMenuItem* menuItems;
    u16 menuItemCount;
    u16 cursorLocation;
    u16 scrollOffset;
};


typedef void (*SetItemValue)(struct CursorMenuItem* item, int id, int value);

struct SelectCursorMenuItem {
    SetItemValue changeCallback;
    int id;
    int value;
    int maxValue;
    int minValue;
    char** labels;
};

void initMenuState(struct MenuState* menu, struct MenuItem* items, int itemCount);

void menuStateHandleInput(struct MenuState* menu, OSContPad* pad);
void menuStateRender(struct MenuState* menu);
void menuItemInit(struct MenuItem* menuItem, void* data, MenuItemRender renderCallback, MenuItemHandleInput handleButtonDown, MenuItemSetActive setActive);

void menuItemConnectSideToSide(struct MenuItem* left, struct MenuItem* right);
void menuItemConnectUpAndDown(struct MenuItem* up, struct MenuItem* down);

struct MenuItem* inputCursorMenu(struct CursorMenu* menu, int buttons, int height);
void renderCursorMenu(struct CursorMenu* menu, int x, int y, int height);
void initCursorMenu(struct CursorMenu* menu, struct CursorMenuItem* menuItems, u16 menuItemCount);
void initCursorMenuItem(struct CursorMenuItem* item, struct MenuItem* toMenu, char* label, u16 height);

void initSelectCursorMenuItem(struct SelectCursorMenuItem* item, int id, int value, int minValue, int maxValue, char** labels);
void renderSelectCursorMenuItem(struct CursorMenuItem* menuItem, int x, int y, int selected);
struct MenuItem* inputSelectCursorMenuItem(struct CursorMenuItem* menuItem, int buttonDown);

void renderSprite(Bitmap* bitmap, s32 x, s32 y, s32 w, s32 h);
void renderMenuBorder();

#endif