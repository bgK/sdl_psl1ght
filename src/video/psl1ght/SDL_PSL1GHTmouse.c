/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2010 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"
#include "SDL_events.h"
#include "../../events/SDL_mouse_c.h"

#include <io/mouse.h>

#include "SDL_PSL1GHTmouse_c.h"

void checkMouseConnected(_THIS) {
    SDL_DeviceData *data =
        (SDL_DeviceData *) _this->driverdata;

    mouseInfo mouseinfo;
    ioMouseGetInfo(&mouseinfo);

    if (mouseinfo.status[0] == 1 && !data->_mouseConnected) // Connected
    {
        data->_mouseConnected = SDL_TRUE;
        data->_mouseButtons = 0;

        // Old events in the queue are discarded
        ioMouseClearBuf(0);
    }
    else if (mouseinfo.status[0] != 1 && data->_mouseConnected) // Disconnected
    {
        data->_mouseConnected = SDL_FALSE;
        data->_mouseButtons = 0;
    }
}

void updateMouseButtons(_THIS, int mouseId, const mouseData *mouse) {
    SDL_DeviceData *data =
        (SDL_DeviceData *) _this->driverdata;
    // There should only be one window
    SDL_Window *window = _this->windows;

    // Check left mouse button changes
    SDL_bool oldLMB = data->_mouseButtons & 1;
    SDL_bool newLMB = mouse->buttons & 1;
    if (newLMB != oldLMB) {
        SDL_SendMouseButton(window, mouseId, newLMB ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
    }

    // Check rigth mouse button changes
    SDL_bool oldRMB = data->_mouseButtons & 2;
    SDL_bool newRMB = mouse->buttons & 2;
    if (newRMB != oldRMB) {
        SDL_SendMouseButton(window, mouseId, newRMB ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT);
    }

    // Check middle mouse button changes
    SDL_bool oldMMB = data->_mouseButtons & 4;
    SDL_bool newMMB = mouse->buttons & 4;
    if (newMMB != oldMMB) {
        SDL_SendMouseButton(window, mouseId, newMMB ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_MIDDLE);
    }

    data->_mouseButtons = mouse->buttons;
}

void updateMousePosition(_THIS, int mouseId, const mouseData *mouse) {
    /* There should only be one window */
    SDL_Window *window = _this->windows;
    SDL_Mouse *sdlMouse = SDL_GetMouse();

    /* Mouse movement is relative */
    int xrel = mouse->x_axis;
    int yrel = mouse->y_axis;

    /* Make sure mouse position stays in the window while still sending the
       event to ensure relative mouse event data is correct for the aplication.
       Modifying last_x and last_y is a gross hack to workaround SDL code
       limitations. */
    int x_max = 0, y_max = 0;
    SDL_GetWindowSize(window, &x_max, &y_max);
    --x_max;
    --y_max;

    if (sdlMouse->last_x + xrel < 0) {
        sdlMouse->last_x = 0 - xrel;
    }

    if (sdlMouse->last_y + yrel < 0) {
        sdlMouse->last_y = 0 - yrel;
    }

    if (sdlMouse->last_x + xrel > x_max) {
        sdlMouse->last_x = x_max - xrel;
    }

    if (sdlMouse->last_y + yrel > y_max) {
        sdlMouse->last_y = y_max - yrel;
    }

    SDL_SendMouseMotion(window, mouseId, 1, xrel, yrel);
}

void updateMouseWheel(_THIS, int mouseId, const mouseData *mouse) {
    // There should only be one window
    SDL_Window *window = _this->windows;

    SDL_SendMouseWheel(window, mouseId, mouse->tilt, mouse->wheel);
}

void
PSL1GHT_PumpMouse(_THIS)
{
    SDL_DeviceData *data =
        (SDL_DeviceData *) _this->driverdata;

    // Check if a mouse has been connected / disconnected
    checkMouseConnected(_this);

    if (data->_mouseConnected)
    {
        mouseDataList datalist;
        ioMouseGetDataList(0, &datalist);

        int i;
        for (i = 0; i < datalist.count; i++) {
            // Send SDL events
            updateMouseButtons(_this, 0, &datalist.list[i]);
            updateMousePosition(_this, 0, &datalist.list[i]);
            updateMouseWheel(_this, 0, &datalist.list[i]);
        }
    }
}

void
PSL1GHT_InitMouse(_THIS)
{
    SDL_DeviceData *data =
        (SDL_DeviceData *) _this->driverdata;

    // Support only one mouse for now
    ioMouseInit(1);

    data->_mouseConnected = SDL_FALSE;
}

void
PSL1GHT_QuitMouse(_THIS)
{
    ioMouseEnd();
}

/* vi: set ts=4 sw=4 expandtab: */
