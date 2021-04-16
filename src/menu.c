
#include "menu.h"
#include "../memory.h"
#include "debug_out.h"
#include "../render.h"
#include "../tex/textures.h"
#include "gameboy.h"

#define BUTTON_ICON_COUNT   10
#define BUTTON_ICON_DL_LENGTH   60

Bitmap gButtonIconTemplates[] = {
    {16, 32, 0, 0, tex_cbuttons, 16, 0},
    {16, 32, 16, 0, tex_cbuttons, 16, 0},
    {16, 32, 0, 16, tex_cbuttons, 16, 0},
    {16, 32, 16, 16, tex_cbuttons, 16, 0},
    
    {16, 32, 0, 0, tex_triggers, 16, 0},
    {16, 32, 16, 0, tex_triggers, 16, 0},
    
    // blank
    {16, 32, 16, 16, tex_triggers, 16, 0},
    {16, 32, 16, 16, tex_triggers, 16, 0},
    
    {16, 32, 16, 0, tex_dpad, 16, 0},
    {16, 32, 0, 0, tex_dpad, 16, 0},
    {16, 32, 0, 16, tex_dpad, 16, 0},
    {16, 32, 16, 16, tex_dpad, 16, 0},
    
    {16, 32, 0, 16, tex_facebuttons, 16, 0},
    {16, 32, 0, 16, tex_triggers, 16, 0},
    {16, 32, 16, 0, tex_facebuttons, 16, 0},
    {16, 32, 0, 0, tex_facebuttons, 16, 0},
};

Bitmap gGUIItemTemplates[] = {
    {8, 32, 0, 0, tex_guiitems, 8, 0},
    {8, 32, 8, 0, tex_guiitems, 8, 0},
    {8, 32, 0, 8, tex_guiitems, 8, 0},
    {8, 32, 8, 8, tex_guiitems, 8, 0},
    {8, 32, 16, 0, tex_guiitems, 8, 0},
    {8, 32, 24, 8, tex_guiitems, 8, 0},
    {8, 32, 24, 0, tex_guiitems, 8, 0},
    {8, 32, 16, 8, tex_guiitems, 8, 0},
    {8, 32, 0, 16, tex_guiitems, 8, 0},
    {8, 32, 8, 16, tex_guiitems, 8, 0},
    {8, 32, 24, 24, tex_guiitems, 8, 0},
};

static Gfx      gButtonIconsDL[NUM_DL(1) * BUTTON_ICON_DL_LENGTH];
static Bitmap   gButtonIcons[BUTTON_ICON_COUNT];

unsigned short gGUIPalette[] = {
	0x0001,
	0xEF2B,
	0xEE87,
	0xBC83,
	0x1,
	0xC631,
	0x6B5D,
	0x2109,
	0x1,
	0xBE3F,
	0xCFB1,
	0x1935,
	0x1E05,
	0x2055,
	0x2C5,
	0xF5EF,
	0xC885,
	0x5045,
	0x1,
	0xEF2B,
	0xFFFF,
	0x0,
	0x1,
	0xC631,
	0x6B5D,
	0x2109,
};

Sprite gButtonSprite = {
    0, 0,
    16, 16,
    1.0, 1.0,
    0, 0,
    SP_TRANSPARENT | SP_CUTOUT,
    0x1234,
    255, 255, 255, 255,
    0, sizeof(gGUIPalette) / sizeof(*gGUIPalette), (int*)gGUIPalette,
    0, 0,
    BUTTON_ICON_COUNT, NUM_DL(1) * BUTTON_ICON_DL_LENGTH,
    32, 32,
    G_IM_FMT_CI,
    G_IM_SIZ_8b,
    gButtonIcons,
    gButtonIconsDL,
    NULL,
};

void renderSprite(Bitmap* bitmap, s32 x, s32 y, s32 w, s32 h)
{
    gButtonSprite.nbitmaps = 1;
    gButtonSprite.bitmap[0] = *bitmap;

    Gfx *gxp, *dl;
    gxp = glistp;
    spMove(&gButtonSprite, x, y);
    spScale(&gButtonSprite, w, h);
    dl = spDraw(&gButtonSprite);
    gSPDisplayList(gxp++, dl);
    glistp = gxp;
}

void setSpriteColor(u8 r, u8 g, u8 b)
{
    gButtonSprite.red = r;
    gButtonSprite.green = g;
    gButtonSprite.blue = b;
}

void initMenuState(struct MenuState* menu, struct MenuItem* items, int itemCount)
{
    zeroMemory(menu, sizeof(struct MenuState));
    menu->currentMenuItem = items;
    menu->allItems = items;
    menu->menuItemCount = itemCount;
    menu->holdTimer = ~0;

    if (menu->currentMenuItem->setActive)
    {
        menu->currentMenuItem->setActive(menu->currentMenuItem, 1);
    }
}

void menuStateSetActive(struct MenuState* menu, struct MenuItem* item)
{
    if (item && item != menu->currentMenuItem)
    {
        if (menu->currentMenuItem->setActive)
        {
            menu->currentMenuItem->setActive(menu->currentMenuItem, 0);
        }
        menu->currentMenuItem = item;
        if (menu->currentMenuItem->setActive)
        {
            menu->currentMenuItem->setActive(menu->currentMenuItem, 1);
        }

    }
}

void menuStateHandleInput(struct MenuState* menu, OSContPad* pad)
{
    if (menu->currentMenuItem)
    {
        int buttons = pad->button;

        if (pad->stick_y > 0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.up);
        if (pad->stick_y < -0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.down);
        if (pad->stick_x > 0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right);
        if (pad->stick_x < -0x40)
            buttons |= INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left);
            

        int buttonDown = (menu->lastButtons ^ buttons) & buttons;

        u32 timeMS = (u32)(OS_CYCLES_TO_USEC(osGetTime()) / 1024);

        if (buttonDown || (menu->holdTimer <= timeMS && buttons))
        {
            if (!buttonDown) {
                buttonDown = buttons;
                menu->holdTimer = timeMS + REPEAT_TIME_DELAY;
            } else {
                menu->holdTimer = timeMS + INITIAL_TIME_DELAY;
            }

            struct MenuItem* nextItem;
            
            if (menu->currentMenuItem->handleButtonDown)
            {
                nextItem = menu->currentMenuItem->handleButtonDown(menu->currentMenuItem, buttonDown, buttons);
            }

            if (nextItem)
            {
                menuStateSetActive(menu, nextItem);
                menu->currentMenuItem = nextItem;
            }
            else
            {
                if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.up)) && menu->currentMenuItem->toUp)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toUp);
                }
                else if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.down)) && menu->currentMenuItem->toDown)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toDown);
                }
                else if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right)) && menu->currentMenuItem->toRight)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toRight);
                }
                else if ((buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left)) && menu->currentMenuItem->toLeft)
                {
                    menuStateSetActive(menu, menu->currentMenuItem->toLeft);
                }
            }
        }

        int buttonUp = (menu->lastButtons ^ buttons) & ~buttons;

        if (buttonUp && menu->currentMenuItem->handleButtonUp)
        {
            menu->holdTimer = ~0;
            menu->currentMenuItem->handleButtonUp(menu->currentMenuItem, buttonUp, buttons);
        }

        menu->lastButtons = buttons;
    }
}

void menuStateRender(struct MenuState* menu)
{
    int index;

    for (index = 0; index < menu->menuItemCount; ++index)
    {
        if (!(menu->allItems[index].flags & MENU_ITEM_FLAGS_HIDDEN))
        {
            menu->allItems[index].renderCallback(&menu->allItems[index], menu->currentMenuItem);
        }
    }
}

void menuItemInit(struct MenuItem* menuItem, void* data, MenuItemRender renderCallback, MenuItemHandleInput handleButtonDown, MenuItemSetActive setActive)
{
    menuItem->data = data;
    menuItem->renderCallback = renderCallback;
    menuItem->handleButtonDown = handleButtonDown;
    menuItem->handleButtonUp = NULL;
    menuItem->setActive = setActive;
}

void menuItemConnectSideToSide(struct MenuItem* left, struct MenuItem* right)
{
    left->toRight = right;
    right->toLeft = left;
}

void menuItemConnectUpAndDown(struct MenuItem* up, struct MenuItem* down)
{
    up->toDown = down;
    down->toUp = up;
}

///////////////////////////////////

void defaultRenderCursorMenuItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    if (selected)
    {
        renderSprite(
            &gGUIItemTemplates[GUIItemIconRight], 
            x, y,
            1, 1
        );
    }

    FONTCOL(255, 255, 255, 255);
    SHOWFONT(&glistp, menuItem->label, x + 12, y);
}

struct MenuItem* defaultInputCursorMenuItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    if (buttonDown & (INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.a) | INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.start)))
    {
        return menuItem->toMenu;
    }

    return NULL;
}

void renderCursorMenu(struct CursorMenu* menu, int x, int y, int height)
{
    int maxY = y + height;
    int menuItemIndex = menu->scrollOffset;

    if (menuItemIndex)
    {
        renderSprite(
            &gGUIItemTemplates[GUIItemIconUp],
            x, y - 8,
            1, 1
        );
    }

    while (menuItemIndex < menu->menuItemCount && y < maxY)
    {
        struct CursorMenuItem* menuItem = &menu->menuItems[menuItemIndex];

        if (y + menuItem->height >= maxY)
        {
            break;
        }

        if (menuItem->render)
        {
            menuItem->render(menuItem, x, y, menuItemIndex == menu->cursorLocation);
        }
        else
        {
            defaultRenderCursorMenuItem(menuItem, x, y, menuItemIndex == menu->cursorLocation);
        }

        y += menuItem->height;
        ++menuItemIndex;
    }

    if (menuItemIndex < menu->menuItemCount)
    {
        renderSprite(
            &gGUIItemTemplates[GUIItemIconDown],
            x, y,
            1, 1
        );
    }
}

struct MenuItem* inputCursorMenu(struct CursorMenu* menu, int buttons, int height)
{
    struct MenuItem* nextItem;
    struct CursorMenuItem* currentItem = &menu->menuItems[menu->cursorLocation];
    
    if (currentItem->input)
    {
        nextItem = currentItem->input(currentItem, buttons);
    }
    else
    {
        nextItem = defaultInputCursorMenuItem(currentItem, buttons);
    }
    
    if (nextItem)
    {
        return nextItem;
    }

    if (buttons & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.up))
    {
        if (menu->cursorLocation > 0)
        {
            if (menu->scrollOffset == menu->cursorLocation)
            {
                --menu->scrollOffset;
            }

            --menu->cursorLocation;
        }
    }

    if (buttons & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.down))
    {
        if (menu->cursorLocation < menu->menuItemCount - 1)
        {
            int lastItemIndex = menu->scrollOffset;
            int y = 0;
            
            while (lastItemIndex < menu->menuItemCount)
            {
                struct CursorMenuItem* cursorMenuItem = &menu->menuItems[lastItemIndex];

                y += cursorMenuItem->height;

                if (y >= height)
                {
                    break;
                }

                ++lastItemIndex;
            }

            if (menu->cursorLocation + 1 >= lastItemIndex)
            {
                ++menu->scrollOffset;
            }

            ++menu->cursorLocation;
        }
    }

    if (buttons & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.b))
    {
        return menu->parentMenu;
    }

    return NULL;
}

void initCursorMenu(struct CursorMenu* menu, struct CursorMenuItem* menuItems, u16 menuItemCount)
{
    zeroMemory(menu, sizeof(struct CursorMenu));
    menu->menuItems = menuItems;
    menu->menuItemCount = menuItemCount;
}

void initCursorMenuItem(struct CursorMenuItem* item, struct MenuItem* toMenu, char* label, u16 height)
{
    zeroMemory(item, sizeof(struct CursorMenuItem));
    item->toMenu = toMenu;
    item->label = label;
    item->height = height;
}

void renderMenuBorder()
{
    renderSprite(&gGUIItemTemplates[GUIItemIconHorz], 0, 40, 12, 1);
    renderSprite(&gGUIItemTemplates[GUIItemIconTopRight], 96, 40, 1, 1);
    renderSprite(&gGUIItemTemplates[GUIItemIconVert], 96, 48, 1, 48);
    renderSprite(&gGUIItemTemplates[GUIItemIconBottomRight], 96, 192, 1, 1);
    renderSprite(&gGUIItemTemplates[GUIItemIconHorz], 0, 192, 12, 1);
    renderSprite(&gGUIItemTemplates[GUIItemIconBlack], 0, 48, 12, 18);
}

///////////////////////////////////

void initSelectCursorMenuItem(struct SelectCursorMenuItem* item, int id, int value, int minValue, int maxValue, char** labels)
{
    item->changeCallback = NULL;
    item->id = id;
    item->value = value;
    item->minValue = minValue;
    item->maxValue = maxValue;
    item->labels = labels;
}

void renderSelectCursorMenuItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    struct SelectCursorMenuItem* select = (struct SelectCursorMenuItem*)menuItem->data;

    FONTCOL(255, 255, 255, 255);
    SHOWFONT(&glistp, menuItem->label, x, y);
    
    if (selected)
    {
        renderSprite(&gGUIItemTemplates[GUIItemIconLeft], x, y + 12, 1, 1);
        renderSprite(&gGUIItemTemplates[GUIItemIconRight], x + 60, y + 12, 1, 1);
    }

    int currentValue = (select->value - select->minValue) % (select->maxValue - select->minValue);
    
    if (select->labels)
    {
        SHOWFONT(&glistp, select->labels[currentValue], x + 12, y + 12);
    }
    else
    {
        char tmpString[16];
        sprintf(tmpString, "%d", select->value);
        SHOWFONT(&glistp, tmpString, x + 12, y + 12);
    }
}

struct MenuItem* inputSelectCursorMenuItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    struct SelectCursorMenuItem* select = (struct SelectCursorMenuItem*)menuItem->data;

    if (buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.right))
    {
        ++select->value;
        
        if (select->value >= select->maxValue)
        {
            select->value = select->minValue;
        }

        if (select->changeCallback)
        {
            select->changeCallback(menuItem, select->id, select->value);
        }
    }

    if (buttonDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.left))
    {
        --select->value;
        
        if (select->value < select->minValue)
        {
            select->value = select->maxValue - 1;
        }

        if (select->changeCallback)
        {
            select->changeCallback(menuItem, select->id, select->value);
        }
    }

    return NULL;
}