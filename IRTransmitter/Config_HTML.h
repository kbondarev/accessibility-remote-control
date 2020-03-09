#ifndef Config_HTML_h
#define Config_HTML_h

#define CONFIG_HTML "<!doctypehtml><html lang=\"en\"><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width,initial-scale=1\"><title>IR-Transmitter Configuration</title><style>body{font-size:1.5em;padding:0 10px 50px 10px}label{display:block;margin-bottom:5px;width:fit-content}[type=button]{background-color:#3a6bd4;border:none;color:#fff;padding:10px 24px;text-align:left;text-decoration:none;display:block;font-size:16px;font-weight:600;width:230px}.green{font-weight:600;padding:0 3px;background-color:#4caf50;color:#fff}.yellow{font-weight:600;padding:0 3px;background-color:#eeb824;color:#fff}.red{font-weight:600;padding:0 3px;background-color:#bd2424;color:#fff}</style><h1>IR Transmitter Configuration</h1><h2>Instructions:</h2><ol><li>Click on the command button you wish to set<li>The button will turn <span class=\"yellow\">yellow</span><li>Point the remote at the device, and then press the corresponding button on the remote<li>The button on the screen will turn <span class=\"green\">green</span> if the device succesfully registered the infrared signal</ol><b>Note that you don't have to configure all the command, but only the ones you are using</b><br><br><br><label><button type=\"button\"onclick=\"clicked(this,1)\">TV POWER ON/OFF</button></label> <label><button type=\"button\"onclick=\"clicked(this,2)\">TV VOLUME UP</button></label> <label><button type=\"button\"onclick=\"clicked(this,3)\">TV VOLUME DOWN</button></label> <label><button type=\"button\"onclick=\"clicked(this,4)\">TV VOLUME MUTE</button></label> <label><button type=\"button\"onclick=\"clicked(this,5)\">TV CHANNEL UP</button></label> <label><button type=\"button\"onclick=\"clicked(this,6)\">TV CHANNEL DOWN</button></label> <label><button type=\"button\"onclick=\"clicked(this,7)\">TV SOURCE</button></label> <label><button type=\"button\"onclick=\"clicked(this,41)\">DVD POWER ON/OFF</button></label> <label><button type=\"button\"onclick=\"clicked(this,42)\">DVD PLAY</button></label> <label><button type=\"button\"onclick=\"clicked(this,43)\">DVD PAUSE</button></label> <label><button type=\"button\"onclick=\"clicked(this,44)\">DVD STOP</button></label> <label><button type=\"button\"onclick=\"clicked(this,45)\">DVD SKIP NEXT</button></label> <label><button type=\"button\"onclick=\"clicked(this,46)\">DVD SKIP PREVIOUS</button></label> <label><button type=\"button\"onclick=\"clicked(this,47)\">DVD MENU</button></label> <label><button type=\"button\"onclick=\"clicked(this,48)\">DVD MENU UP</button></label> <label><button type=\"button\"onclick=\"clicked(this,49)\">DVD MENU DOWN</button></label> <label><button type=\"button\"onclick=\"clicked(this,50)\">DVD MENU LEFT</button></label> <label><button type=\"button\"onclick=\"clicked(this,51)\">DVD MENU RIGHT</button></label> <label><button type=\"button\"onclick=\"clicked(this,52)\">DVD MENU ENTER</button></label> <label><button type=\"button\"onclick=\"clicked(this,53)\">DVD VOLUME UP</button></label> <label><button type=\"button\"onclick=\"clicked(this,54)\">DVD VOLUME DOWN</button></label> <label><button type=\"button\"onclick=\"clicked(this,55)\">DVD VOLUME MUTE</button></label><script>const GREEN = '#4caf50';\n\r\
    const YELLOW = '#eeb824';\n\r\
    const BLUE = '#3a6bd4';\n\r\
    function clicked(button, command) {\n\r\
      //   let text = new String(el.innerHTML);\n\r\
      button.style.backgroundColor = YELLOW;\n\r\
      doPost(command, button);\n\r\
    }\n\r\
    function doPost(command, button) {\n\r\
      var xhr = new XMLHttpRequest();\n\r\
      xhr.open('GET', './cmd/'+command);\n\r\
      xhr.setRequestHeader('Content-Type', 'text/plain');\n\r\
      xhr.onload = function() {\n\r\
        if (xhr.status === 200) {\n\r\
          alert('Infrared signal was saved succesfully!');\n\r\
          button.style.backgroundColor = GREEN;\n\r\
        } else {\n\r\
          alert('Failed to set the infrared signal');\n\r\
          button.style.backgroundColor = BLUE;\n\r\
        }\n\r\
      };\n\r\
      xhr.send();\n\r\
    }</script>\n\r\
    "

#endif Config_HTML_h
