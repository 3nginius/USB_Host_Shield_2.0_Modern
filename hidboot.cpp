/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
 */
#include "hidboot.h"

void MouseReportParser::Parse(USBHID *hid __attribute__((unused)), bool is_rpt_id __attribute__((unused)), uint8_t len __attribute__((unused)), uint8_t *buf) {
        MOUSEINFO *pmi = (MOUSEINFO*)buf;
        // Future:
        // bool event;
 
#if 0
        if (prevState.mouseInfo.bmLeftButton == 0 && pmi->bmLeftButton == 1)
                OnLeftButtonDown(pmi);
 
        if (prevState.mouseInfo.bmLeftButton == 1 && pmi->bmLeftButton == 0)
                OnLeftButtonUp(pmi);
 
        if (prevState.mouseInfo.bmRightButton == 0 && pmi->bmRightButton == 1)
                OnRightButtonDown(pmi);
 
        if (prevState.mouseInfo.bmRightButton == 1 && pmi->bmRightButton == 0)
                OnRightButtonUp(pmi);
 
        if (prevState.mouseInfo.bmMiddleButton == 0 && pmi->bmMiddleButton == 1)
                OnMiddleButtonDown(pmi);
 
        if (prevState.mouseInfo.bmMiddleButton == 1 && pmi->bmMiddleButton == 0)
                OnMiddleButtonUp(pmi);
 
        if (prevState.mouseInfo.bmXButton1 == 0 && pmi->bmXButton1 == 1)
                OnXB1ButtonDown(pmi);
 
        if (prevState.mouseInfo.bmXButton1 == 1 && pmi->bmXButton1 == 0)
                OnXB1ButtonUp(pmi);
 
        if (prevState.mouseInfo.bmXButton2 == 0 && pmi->bmXButton2 == 1)
                OnXB2ButtonDown(pmi);
 
        if (prevState.mouseInfo.bmXButton2 == 1 && pmi->bmXButton2 == 0)
                OnXB2ButtonUp(pmi);
 
        if (prevState.mouseInfo.dX != pmi->dX || prevState.mouseInfo.dY != pmi->dY)
                OnMouseMove(pmi);
 
        if (len > sizeof (MOUSEINFO))
                for (uint8_t i = 0; i<sizeof (MOUSEINFO); i++)
                        prevState.bInfo[i] = buf[i];
#else
        if(prevState.mouseInfo.bmLeftButton != pmi->bmLeftButton ) {
                if(pmi->bmLeftButton) {
                        OnLeftButtonDown(pmi);
                } else {
                        OnLeftButtonUp(pmi);
                }
                // Future:
                // event = true;
        }
 
        if(prevState.mouseInfo.bmRightButton != pmi->bmRightButton) {
                if(pmi->bmRightButton) {
                        OnRightButtonDown(pmi);
                } else {
                        OnRightButtonUp(pmi);
                }
                // Future:
                // event = true;
        }
 
        if(prevState.mouseInfo.bmMiddleButton != pmi->bmMiddleButton) {
                if(pmi->bmMiddleButton) {
                        OnMiddleButtonDown(pmi);
                } else {
                        OnMiddleButtonUp(pmi);
                }
                // Future:
                // event = true;
        }
 
        if(prevState.mouseInfo.bmXButton1 != pmi->bmXButton1) {
                if(pmi->bmXButton1) {
                        OnXB1ButtonDown(pmi);
                } else {
                        OnXB1ButtonUp(pmi);
                }
                // Future:
                // event = true;
        }
 
        if(prevState.mouseInfo.bmXButton2 != pmi->bmXButton2) {
                if(pmi->bmXButton2) {
                        OnXB2ButtonDown(pmi);
                } else {
                        OnXB2ButtonUp(pmi);
                }
                // Future:
                // event = true;
        }
 
        if(pmi->dZ) {
                OnMouseScroll(pmi);
                // Future:
                // event = true;
        }
 
        // Mice only report motion when they actually move!
        // Why not just pass the x/y values to simplify things??
        if(pmi->dX || pmi->dY) {
                OnMouseMove(pmi);
                // Future:
                // event = true;
        }
 
        //
        // Future:
        // Provide a callback that operates on the gathered events from above.
        //
        // if(event) OnMouse();
        //
 
        // Only the first byte matters (buttons). We do NOT need to save position info.
        prevState.bInfo[0] = buf[0];
#endif
 
};

void KeyboardReportParser::Parse(USBHID *hid, bool is_rpt_id __attribute__((unused)), uint8_t len __attribute__((unused)), uint8_t *buf) {
        // On error - return
        if (buf[2] == 1)
                return;

        //KBDINFO       *pki = (KBDINFO*)buf;

        // provide event for changed control key state
        if (prevState.bInfo[0x00] != buf[0x00]) {
                OnControlKeysChanged(prevState.bInfo[0x00], buf[0x00]);
        }

        for (uint8_t i = 2; i < 8; i++) {
                bool down = false;
                bool up = false;

                for (uint8_t j = 2; j < 8; j++) {
                        if (buf[i] == prevState.bInfo[j] && buf[i] != 1)
                                down = true;
                        if (buf[j] == prevState.bInfo[i] && prevState.bInfo[i] != 1)
                                up = true;
                }
                if (!down) {
                        HandleLockingKeys(hid, buf[i]);
                        OnKeyDown(*buf, buf[i]);
                }
                if (!up)
                        OnKeyUp(prevState.bInfo[0], prevState.bInfo[i]);
        }
        for (uint8_t i = 0; i < 8; i++)
                prevState.bInfo[i] = buf[i];
};

const uint8_t KeyboardReportParser::numKeys[10] PROGMEM = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
const uint8_t KeyboardReportParser::symKeysUp[12] PROGMEM = {'_', '+', '{', '}', '|', '~', ':', '"', '~', '<', '>', '?'};
const uint8_t KeyboardReportParser::symKeysLo[12] PROGMEM = {'-', '=', '[', ']', '\\', ' ', ';', '\'', '`', ',', '.', '/'};
const uint8_t KeyboardReportParser::padKeys[5] PROGMEM = {'/', '*', '-', '+', '\r'};

uint8_t KeyboardReportParser::OemToAscii(uint8_t mod, uint8_t key) {
        uint8_t shift = (mod & 0x22);

        // [a-z]
        if (VALUE_WITHIN(key, 0x04, 0x1d)) {
                // Upper case letters
                if ((kbdLockingKeys.kbdLeds.bmCapsLock == 0 && shift) ||
                        (kbdLockingKeys.kbdLeds.bmCapsLock == 1 && shift == 0))
                        return (key - 4 + 'A');

                        // Lower case letters
                else
                        return (key - 4 + 'a');
        }// Numbers
        else if (VALUE_WITHIN(key, 0x1e, 0x27)) {
                if (shift)
                        return ((uint8_t)pgm_read_byte(&getNumKeys()[key - 0x1e]));
                else
                        return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
        }// Keypad Numbers
        else if(VALUE_WITHIN(key, 0x59, 0x61)) {
                if(kbdLockingKeys.kbdLeds.bmNumLock == 1)
                        return (key - 0x59 + '1');
        } else if(VALUE_WITHIN(key, 0x2d, 0x38))
                return ((shift) ? (uint8_t)pgm_read_byte(&getSymKeysUp()[key - 0x2d]) : (uint8_t)pgm_read_byte(&getSymKeysLo()[key - 0x2d]));
        else if(VALUE_WITHIN(key, 0x54, 0x58))
                return (uint8_t)pgm_read_byte(&getPadKeys()[key - 0x54]);
        else {
                switch(key) {
                        case UHS_HID_BOOT_KEY_SPACE: return (0x20);
                        case UHS_HID_BOOT_KEY_ENTER: return ('\r'); // Carriage return (0x0D)
                        case UHS_HID_BOOT_KEY_ZERO2: return ((kbdLockingKeys.kbdLeds.bmNumLock == 1) ? '0': 0);
                        case UHS_HID_BOOT_KEY_PERIOD: return ((kbdLockingKeys.kbdLeds.bmNumLock == 1) ? '.': 0);
                }
        }
        return ( 0);
}
