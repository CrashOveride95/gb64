
#include "inputmapping.h"
#include "gameboy.h"
#include "render.h"
#include "sprite.h"
#include "spritefont.h"

static char* gInputMappingNames[MAPPED_INPUT_COUNT] = {
    "RGHT",
    "LEFT",
    "UP",
    "DOWN",
    "A",
    "B",
    "SLCT",
    "STRT",
    "SAVE",
    "LOAD",
    "MENU",
    "FAST",
};

void renderInputMappingItem(struct CursorMenuItem* menuItem, int x, int y, int selected)
{
    enum InputButtonIndex buttonIndex = (enum InputButtonIndex)menuItem->data;

    if (selected)
    {
        spriteDrawTile(
            SPRITE_BORDER_LAYER,
            x, y + 4,
            8, 8,
            gGUIItemTiles[GUIItemIconRight]
        );
    }

    if (buttonIndex != ~0)
    {
        enum InputButtonSetting buttonSetting = getButtonMapping(&gGameboy.settings.inputMapping, buttonIndex);

        spriteDrawTile(
            gButtonIconLayer[buttonSetting],
            x + 12, y,
            16, 16,
            gButtonIconTile[buttonSetting]
        );
    }

    spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
    renderText(&gGBFont, menuItem->label, x + 32, y + 4, 0);
}

struct MenuItem* inputInputMappingItem(struct CursorMenuItem* menuItem, int buttonDown)
{
    return NULL;
}


void inputMappingRender(struct MenuItem* menuItem, struct MenuItem* highlightedItem)
{
    struct InputMappingMenu* inputMapping = (struct InputMappingMenu*)menuItem->data;
    
    if (menuItem == highlightedItem)
    {
        renderMenuBorder();
        spriteSetColor(gGBFont.spriteLayer, 255, 255, 255, 255);
        renderText(&gGBFont, "INPUT", 16, 32, 0);
        renderCursorMenu(&inputMapping->cursor, 16, 56, 136);
    }
}

struct MenuItem* inputMappingHandleInput(struct MenuItem* menuItem, int buttonsDown, int buttonsState)
{
    struct InputMappingMenu* inputMapping = (struct InputMappingMenu*)menuItem->data;
    
    if (inputMapping->currentMappingIndex != ~0 && buttonsDown)
    {
        int setting = 0;

        while (buttonsDown & ~0x1)
        {
            ++setting;
            buttonsDown >>= 1;
        }

        setButtonMapping(&gGameboy.settings.inputMapping, inputMapping->currentMappingIndex, setting);
        
        inputMapping->cursor.menuItems[inputMapping->currentMappingIndex].data = (void*)inputMapping->currentMappingIndex;
        inputMapping->currentMappingIndex = ~0;
        return menuItem;
    }
    else if (buttonsDown & (START_BUTTON | A_BUTTON))
    {
        inputMapping->currentMappingIndex = inputMapping->cursor.cursorLocation;
        inputMapping->cursor.menuItems[inputMapping->cursor.cursorLocation].data = (void*)~0;
        return menuItem;
    }
    else if (buttonsDown & INPUT_BUTTON_TO_MASK(gGameboy.settings.inputMapping.openMenu))
    {
        return inputMapping->cursor.parentMenu;
    }
    else
    {
        struct MenuItem* result = inputCursorMenu(&inputMapping->cursor, buttonsDown, 136);

        if (result)
        {
            return result;
        }
        else
        {
            return menuItem;
        }
    }
}

void initInputMappingMenu(struct InputMappingMenu* menu, struct MenuItem* parentMenu)
{
    initCursorMenu(
        &menu->cursor,
        menu->menuItems,
        MAPPED_INPUT_COUNT
    );
    menu->cursor.parentMenu = parentMenu;
    menu->currentMappingIndex = ~0;

    int i;

    for (i = 0; i < MAPPED_INPUT_COUNT; ++i)
    {
        initCursorMenuItem(
            &menu->menuItems[i],
            NULL,
            gInputMappingNames[i],
            20
        );
        menu->menuItems[i].data = (void*)i;
        menu->menuItems[i].render = renderInputMappingItem;
        menu->menuItems[i].input = inputInputMappingItem;
    }
}