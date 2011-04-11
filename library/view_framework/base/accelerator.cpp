
#include "accelerator.h"

#include "base/logging.h"
#include "base/rtl.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"

#include "../base/app_res_ids.h"
#include "../l10n/l10n_util.h"

namespace view
{

    string16 Accelerator::GetShortcutText() const
    {
        int string_id = 0;
        switch(key_code_)
        {
        case VKEY_TAB:
            string_id = IDS_APP_TAB_KEY;
            break;
        case VKEY_RETURN:
            string_id = IDS_APP_ENTER_KEY;
            break;
        case VKEY_ESCAPE:
            string_id = IDS_APP_ESC_KEY;
            break;
        case VKEY_PRIOR:
            string_id = IDS_APP_PAGEUP_KEY;
            break;
        case VKEY_NEXT:
            string_id = IDS_APP_PAGEDOWN_KEY;
            break;
        case VKEY_END:
            string_id = IDS_APP_END_KEY;
            break;
        case VKEY_HOME:
            string_id = IDS_APP_HOME_KEY;
            break;
        case VKEY_INSERT:
            string_id = IDS_APP_INSERT_KEY;
            break;
        case VKEY_DELETE:
            string_id = IDS_APP_DELETE_KEY;
            break;
        case VKEY_LEFT:
            string_id = IDS_APP_LEFT_ARROW_KEY;
            break;
        case VKEY_RIGHT:
            string_id = IDS_APP_RIGHT_ARROW_KEY;
            break;
        case VKEY_BACK:
            string_id = IDS_APP_BACKSPACE_KEY;
            break;
        case VKEY_F1:
            string_id = IDS_APP_F1_KEY;
            break;
        case VKEY_F11:
            string_id = IDS_APP_F11_KEY;
            break;
        default:
            break;
        }

        string16 shortcut;
        if(!string_id)
        {
            // Our fallback is to try translate the key code to a regular character
            // unless it is one of digits (VK_0 to VK_9). Some keyboard
            // layouts have characters other than digits assigned in
            // an unshifted mode (e.g. French AZERY layout has 'a with grave
            // accent' for '0'). For display in the menu (e.g. Ctrl-0 for the
            // default zoom level), we leave VK_[0-9] alone without translation.
            wchar_t key;
            if(key_code_>='0' && key_code_<='9')
            {
                key = key_code_;
            }
            else
            {
#ifndef MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR (2)
#endif
                key = LOWORD(::MapVirtualKeyW(key_code_, MAPVK_VK_TO_CHAR));
            }
            shortcut += key;
        }
        else
        {
            shortcut = GetStringUTF16(string_id);
        }

        // Checking whether the character used for the accelerator is alphanumeric.
        // If it is not, then we need to adjust the string later on if the locale is
        // right-to-left. See below for more information of why such adjustment is
        // required.
        string16 shortcut_rtl;
        bool adjust_shortcut_for_rtl = false;
        if(base::IsRTL() && shortcut.length()==1 &&
            !IsAsciiAlpha(shortcut.at(0)) && !IsAsciiDigit(shortcut.at(0)))
        {
            adjust_shortcut_for_rtl = true;
            shortcut_rtl.assign(shortcut);
        }

        if(IsShiftDown())
        {
            shortcut = GetStringFUTF16(IDS_APP_SHIFT_MODIFIER, shortcut);
        }

        // Note that we use 'else-if' in order to avoid using Ctrl+Alt as a shortcut.
        // See http://blogs.msdn.com/oldnewthing/archive/2004/03/29/101121.aspx for
        // more information.
        if(IsCtrlDown())
        {
            shortcut = GetStringFUTF16(IDS_APP_CONTROL_MODIFIER, shortcut);
        }
        else if(IsAltDown())
        {
            shortcut = GetStringFUTF16(IDS_APP_ALT_MODIFIER, shortcut);
        }

        // For some reason, menus in Windows ignore standard Unicode directionality
        // marks (such as LRE, PDF, etc.). On RTL locales, we use RTL menus and
        // therefore any text we draw for the menu items is drawn in an RTL context.
        // Thus, the text "Ctrl++" (which we currently use for the Zoom In option)
        // appears as "++Ctrl" in RTL because the Unicode BiDi algorithm puts
        // punctuations on the left when the context is right-to-left. Shortcuts that
        // do not end with a punctuation mark (such as "Ctrl+H" do not have this
        // problem).
        //
        // The only way to solve this problem is to adjust the string if the locale
        // is RTL so that it is drawn correnctly in an RTL context. Instead of
        // returning "Ctrl++" in the above example, we return "++Ctrl". This will
        // cause the text to appear as "Ctrl++" when Windows draws the string in an
        // RTL context because the punctunation no longer appears at the end of the
        // string.
        //
        // TODO(idana) bug# 1232732: this hack can be avoided if instead of using
        // view::Menu we use view::MenuItemView because the latter is a View
        // subclass and therefore it supports marking text as RTL or LTR using
        // standard Unicode directionality marks.
        if(adjust_shortcut_for_rtl)
        {
            int key_length = static_cast<int>(shortcut_rtl.length());
            DCHECK_GT(key_length, 0);
            shortcut_rtl.append(ASCIIToUTF16("+"));

            // Subtracting the size of the shortcut key and 1 for the '+' sign.
            shortcut_rtl.append(shortcut, 0, shortcut.length()-key_length-1);
            shortcut.swap(shortcut_rtl);
        }

        return shortcut;
    }

} //namespace view