#ifndef Config_HTML_h
#define Config_HTML_h

#define CONFIG_HTML "<!DOCTYPE html>\
<html lang=\"en\">\
        <head>\
            <meta charset=\"UTF-8\" />\
            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\
            <title>IR-Tx Settings</title>\
            <style>\
            form {\
                width: 400px;\
                position: relative;\
            }\
            label {\
                display: block;\
                margin-top: 16px;\
                position: relative;\
            }\
            label > * {\
                position: absolute;\
                right: 0;\
            }\
            select {\
                width: 160px;\
            }\
            button {\
                position: absolute;\
                right: 0;\
            }\
        </style>\
    </head>\
    <body>\
            <h2>IR-Tx Settings</h2>\
    \
            <form action=\"/submit\" method=\"get\">\
    \
                <label for=\"protocol\">PROTOCOL:\
                    <select id=\"protocol\" name=\"protocol\">\
                        <option value=\"nec\">NEC</option>\
                        <option value=\"sony\">Sony</option>\
                </select>\
            </label>\
    \
                <label for=\"TV_POWER\">TV_POWER:\
                    <input type=\"text\" id=\"TV_POWER\" name=\"TV_POWER\" />\
            </label>\
                <label for=\"TV_VOL_UP\">TV_VOL_UP:\
                    <input type=\"text\" id=\"TV_VOL_UP\" name=\"TV_VOL_UP\" />\
            </label>\
    \
                <label for=\"DVD_POWER\">DVD_POWER:\
                    <input type=\"text\" id=\"DVD_POWER\" name=\"DVD_POWER\" />\
            </label>\
                <label for=\"DVD_PLAY\">DVD_PLAY:\
                    <input type=\"text\" id=\"DVD_PLAY\" name=\"DVD_PLAY\" />\
            </label>\
                \
                <label for=\"DVD_MENU\">DVD_MENU:\
                    <input type=\"text\" id=\"DVD_MENU\" name=\"DVD_MENU\" />\
            </label>\
                <label for=\"DVD_MENU_UP\">DVD_MENU_UP:\
                    <input type=\"text\" id=\"DVD_MENU_UP\" name=\"DVD_MENU_UP\" />\
            </label>\
                <label for=\"DVD_MENU_DOWN\">DVD_MENU_DOWN:\
                    <input type=\"text\" id=\"DVD_MENU_DOWN\" name=\"DVD_MENU_DOWN\" />\
            </label>\
                <label for=\"DVD_MENU_LEFT\">DVD_MENU_LEFT:\
                    <input type=\"text\" id=\"DVD_MENU_LEFT\" name=\"DVD_MENU_LEFT\" />\
            </label>\
                <label for=\"DVD_MENU_RIGHT\">DVD_MENU_RIGHT:\
                    <input type=\"text\" id=\"DVD_MENU_RIGHT\" name=\"DVD_MENU_RIGHT\" />\
            </label>\
                <label for=\"DVD_MENU_ENTER\">DVD_MENU_ENTER:\
                    <input type=\"text\" id=\"DVD_MENU_ENTER\" name=\"DVD_MENU_ENTER\" />\
            </label>\
                \
                <br />\
                <button type=\"submit\">Submit</button>\
        </form>\
    </body>\
</html>"



#endif BLE_Protocol_h
