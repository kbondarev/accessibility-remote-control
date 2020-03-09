#ifndef Config_HTML_h
#define Config_HTML_h
#define CONFIG_HTML "<!doctypehtml><html lang=\"en\"><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width,initial-scale=1\"><title>Trigger Configuration</title><style>body{padding:0 10px 50px 10px}select{margin:20px 0 30px 0;font-size:1.3em}#save{margin-top:20px}#add,#save{background-color:#db690c;border:none;color:#fff;padding:5px 14px;text-align:left;text-decoration:none;font-size:1.2em;font-weight:700}table{font-family:'Trebuchet MS',Arial,Helvetica,sans-serif;border-collapse:collapse;width:100%}table td,table th{border:1px solid #ddd;padding:8px}table tr:nth-child(even){background-color:#f2f2f2}table tr:hover{background-color:#ddd}table th{padding-top:12px;padding-bottom:12px;text-align:left;background-color:#4caf50;color:#fff}.btn{border:none;color:#fff;margin:0 2px;text-align:center;text-decoration:none;font-size:24px;font-weight:800;display:inline-block;cursor:pointer}.btn.arrow{background-color:#4caf50}.btn.delete{background-color:#c31414}:disabled{background-color:grey!important}</style><h1>IR Transmitter Configuration</h1><h2>Instructions:</h2><ol><li>The commands will be executed in the order they appear in the table (top command will be executed first, while the bottom command last)<li>To add a command, select it from the dropdown menu and click on the <b>ADD COMMAND</b> button<li>You may use the action buttons to move the commands <b>UP</b> or <b>DOWN</b>, or <b>DELETE</b> it from the list<li>Once you are done, click the <b>SAVE</b> button at the bottom of the table</ol><br><br><br><select id=\"select\"></select> <input id=\"add\"type=\"button\"value=\"ADD COMMAND\"onclick=\"clickedAdd()\"><table><thead><tr><th>#<th>COMMAND<th>ACTIONS<tbody id=\"table\"></table><input id=\"save\"type=\"button\"value=\"SAVE\"onclick=\"clickedAdd()\"><script>const GREEN = '#4caf50';\
    const YELLOW = '#eeb824';\
    const BLUE = '#3a6bd4';\
    const OPTION = '<option value=\"$$VAL$$\">$$CMD$$</option>';\
    const ROW = `<tr>\
        <td>$$INDEX$$</td>\
        <td>$$CMD$$</td>\
        <td>\
          <input\
            class=\"btn arrow\"\
            type=\"button\"\
            value=\"&uarr;\"\
            $$UP$$\
            onclick=\"clickedUp($$INDEX$$)\"\
          />\
          <input\
            class=\"btn arrow\"\
            type=\"button\"\
            value=\"&darr;\"\
            $$DOWN$$\
            onclick=\"clickedDown($$INDEX$$)\"\
          />\
          <input\
            class=\"btn delete\"\
            type=\"button\"\
            value=\"x\"\
            onclick=\"clickedDelete($$INDEX$$)\"\
          />\
        </td>\
      </tr>`;\
    const ALL_COMMANDS = [\
      [1, 'TV_POWER'],\
      [2, 'TV_VOL_UP'],\
      [3, 'TV_VOL_DOWN'],\
      [4, 'TV_VOL_MUTE'],\
      [5, 'TV_CHANNEL_UP'],\
      [6, 'TV_CHANNEL_DOWN'],\
      [7, 'TV_SOURCE'],\
      [41, 'DVD_POWER'],\
      [42, 'DVD_PLAY'],\
      [43, 'DVD_PAUSE'],\
      [44, 'DVD_STOP'],\
      [45, 'DVD_SKIP_NEXT'],\
      [46, 'DVD_SKIP_PREV'],\
      [47, 'DVD_MENU'],\
      [48, 'DVD_MENU_UP'],\
      [49, 'DVD_MENU_DOWN'],\
      [50, 'DVD_MENU_LEFT'],\
      [51, 'DVD_MENU_RIGHT'],\
      [52, 'DVD_MENU_ENTER'],\
      [53, 'DVD_VOL_UP'],\
      [54, 'DVD_VOL_DOWN'],\
      [55, 'DVD_VOL_MUTE']\
    ];\
    let commands = [];\
    function renderSelectOptions() {\
      let content = '';\
      ALL_COMMANDS.forEach((cmd, i) => {\
        let opt = new String(OPTION);\
        opt = opt.replace('$$VAL$$', i);\
        opt = opt.replace('$$CMD$$', cmd[1]);\
        content += opt;\
      });\
      let select = document.getElementById('select');\
      select.innerHTML = content;\
    }\
    function renderCommands() {\
      let content = '';\
      commands.forEach((cmd, i) => {\
        let row = new String(ROW);\
        row = row.replace('$$CMD$$', cmd[1]);\
        row = row.replace('$$INDEX$$', i + 1);\
        row = row.replace('$$INDEX$$', i);\
        row = row.replace('$$INDEX$$', i);\
        row = row.replace('$$INDEX$$', i);\
        row = row.replace('$$UP$$', i == 0 ? 'disabled' : '');\
        row = row.replace(\
          '$$DOWN$$',\
          i == commands.length - 1 ? 'disabled' : ''\
        );\
        content += row;\
      });\
      if (commands.length == 0) {\
        content = '<tr><td colspan=\"3\" style=\"text-align: center; font-size: 2em; font-weight: 700;\">NO COMMANDS SELECTED!</td></tr>'\
      }\
      let table = document.getElementById('table');\
      table.innerHTML = content;\
    }\
    function clickedAdd() {\
      let select = document.getElementById('select');\
      commands.push(ALL_COMMANDS[select.value]);\
      renderCommands();\
    }\
    function clickedUp(index) {\
      console.log('up: index=', index);\
      var tmp = commands.splice(index, 1)[0];\
      commands.splice(index - 1, 0, tmp);\
      console.log('up: commands=', commands);\
      renderCommands();\
    }\
    function clickedDown(index) {\
      console.log('down: index=', index);\
      var tmp = commands.splice(index, 1)[0];\
      commands.splice(index + 1, 0, tmp);\
      console.log('down: commands=', commands);\
      renderCommands();\
    }\
    function clickedDelete(index) {\
      commands.splice(index, 1)[0];\
      renderCommands();\
    }\
    function doPost(command, button) {\
      var xhr = new XMLHttpRequest();\
      xhr.open('GET', './cmd/' + command);\
      xhr.setRequestHeader('Content-Type', 'text/plain');\
      xhr.onload = function() {\
        if (xhr.status === 200) {\
          alert('Infrared signal was saved succesfully!');\
          button.style.backgroundColor = GREEN;\
        } else {\
          alert('Failed to set the infrared signal');\
          button.style.backgroundColor = BLUE;\
        }\
      };\
      xhr.send();\
    }\
    renderSelectOptions();\
    renderCommands();</script>\"
#endif BLE_Protocol_h