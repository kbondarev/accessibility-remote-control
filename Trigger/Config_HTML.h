#ifndef Config_HTML_h
#define Config_HTML_h

#define CONFIG_HTML1 "<!doctypehtml><html lang=\"en\"><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width,initial-scale=1\"><title>Trigger Configuration</title><style>body{padding:0 10px 50px 10px}select{margin:20px 0 30px 0;font-size:1.3em}#save{margin-top:20px}#add,#save{background-color:#db690c;border:none;color:#fff;padding:5px 14px;text-align:left;text-decoration:none;font-size:1.2em;font-weight:700}table{font-family:'Trebuchet MS',Arial,Helvetica,sans-serif;border-collapse:collapse;width:100%}table td,table th{border:1px solid #ddd;padding:8px}table tr:nth-child(even){background-color:#f2f2f2}table tr:hover{background-color:#ddd}table th{padding-top:12px;padding-bottom:12px;text-align:left;background-color:#4caf50;color:#fff}.btn{border:none;color:#fff;margin:0 2px;text-align:center;text-decoration:none;font-size:24px;font-weight:800;display:inline-block;cursor:pointer}.btn.arrow{background-color:#4caf50}.btn.delete{background-color:#c31414}:disabled{background-color:grey!important}</style><h1>IR Transmitter Configuration</h1><h2>Instructions:</h2><ol><li>The commands will be executed in the order they appear in the table (top command will be executed first, while the bottom command last)<li>To add a command, select it from the dropdown menu and click on the <b>ADD COMMAND</b> button<li>You may use the action buttons to move the commands <b>UP</b> or <b>DOWN</b>, or <b>DELETE</b> it from the list<li>Once you are done, click the <b>SAVE</b> button at the bottom of the table</ol><br><br><br><select id=\"select\"></select> <input id=\"add\"type=\"button\"value=\"ADD COMMAND\"onclick=\"clickedAdd()\"><table><thead><tr><th>#<th>COMMAND<th>ACTIONS<tbody id=\"table\"></table><input id=\"save\"type=\"button\"value=\"SAVE\"onclick=\"clickedSave()\"><script>const GREEN = '#4caf50';\n\r\
    const YELLOW = '#eeb824';\n\r\
    const BLUE = '#3a6bd4';\n\r\
    const OPTION = '<option value=\"$$VAL$$\">$$CMD$$</option>';\n\r\
    const ROW = `<tr>\n\r\
        <td>$$INDEX$$</td>\n\r\
        <td>$$CMD$$</td>\n\r\
        <td>\n\r\
          <input\n\r\
            class=\"btn arrow\"\n\r\
            type=\"button\"\n\r\
            value=\"&uarr;\"\n\r\
            $$UP$$\n\r\
            onclick=\"clickedUp($$INDEX$$)\"\n\r\
          />\n\r\
          <input\n\r\
            class=\"btn arrow\"\n\r\
            type=\"button\"\n\r\
            value=\"&darr;\"\n\r\
            $$DOWN$$\n\r\
            onclick=\"clickedDown($$INDEX$$)\"\n\r\
          />\n\r\
          <input\n\r\
            class=\"btn delete\"\n\r\
            type=\"button\"\n\r\
            value=\"x\"\n\r\
            onclick=\"clickedDelete($$INDEX$$)\"\n\r\
          />\n\r\
        </td>\n\r\
      </tr>`;\n\r\
    const ALL_COMMANDS = [\n\r\
      [255, 'WAIT 3 SECONDS'],\n\r\
      [1, 'TV POWER'],\n\r\
      [2, 'TV VOLUME UP'],\n\r\
      [3, 'TV VOLUME DOWN'],\n\r\
      [4, 'TV VOLVOLUME MUTE'],\n\r\
      [5, 'TV CHANNEL UP'],\n\r\
      [6, 'TV CHANNEL DOWN'],\n\r\
      [7, 'TV SOURCE'],\n\r\
      [41, 'DVD POWER'],\n\r\
      [42, 'DVD PLAY'],\n\r\
      [43, 'DVD PAUSE'],\n\r\
      [44, 'DVD STOP'],\n\r\
      [45, 'DVD SKIP NEXT'],\n\r\
      [46, 'DVD SKIP PREV'],\n\r\
      [47, 'DVD MENU'],\n\r\
      [48, 'DVD MENU UP'],\n\r\
      [49, 'DVD MENU DOWN'],\n\r\
      [50, 'DVD MENU LEFT'],\n\r\
      [51, 'DVD MENU RIGHT'],\n\r\
      [52, 'DVD MENU ENTER'],\n\r\
      [53, 'DVD VOLUME UP'],\n\r\
      [54, 'DVD VOLUME DOWN'],\n\r\
      [55, 'DVD VOLUME MUTE']\n\r\
    ];\n\r\
    let commands = [];\n\r\
    function renderSelectOptions() {\n\r\
      let content = '';\n\r\
      ALL_COMMANDS.forEach((cmd, i) => {\n\r\
        let opt = new String(OPTION);\n\r\
        opt = opt.replace('$$VAL$$', i);\n\r\
        opt = opt.replace('$$CMD$$', cmd[1]);\n\r\
        content += opt;\n\r\
      });\n\r\
      let select = document.getElementById('select');\n\r\
      select.innerHTML = content;\n\r\
    }\n\r"
    
#define CONFIG_HTML2 "function renderCommands() {\n\r\
      let content = '';\n\r\
      commands.forEach((cmd, i) => {\n\r\
        let row = new String(ROW);\n\r\
        row = row.replace('$$CMD$$', cmd[1]);\n\r\
        row = row.replace('$$INDEX$$', i + 1);\n\r\
        row = row.replace('$$INDEX$$', i);\n\r\
        row = row.replace('$$INDEX$$', i);\n\r\
        row = row.replace('$$INDEX$$', i);\n\r\
        row = row.replace('$$UP$$', i == 0 ? 'disabled' : '');\n\r\
        row = row.replace(\n\r\
          '$$DOWN$$',\n\r\
          i == commands.length - 1 ? 'disabled' : ''\n\r\
        );\n\r\
        content += row;\n\r\
      });\n\r\
      if (commands.length == 0) {\n\r\
        content = '<tr><td colspan=\"3\" style=\"text-align: center; font-size: 2em; font-weight: 700;\">NO COMMANDS SELECTED!</td></tr>'\n\r\
      }\n\r\
      let table = document.getElementById('table');\n\r\
      table.innerHTML = content;\n\r\
    }\n\r\
    function clickedAdd() {\n\r\
      let select = document.getElementById('select');\n\r\
      commands.push(ALL_COMMANDS[select.value]);\n\r\
      renderCommands();\n\r\
    }\n\r\
    function clickedSave() {\n\r\
      doGET();\n\r\
    }\n\r\
    function clickedUp(index) {\n\r\
      console.log('up: index=', index);\n\r\
      var tmp = commands.splice(index, 1)[0];\n\r\
      commands.splice(index - 1, 0, tmp);\n\r\
      console.log('up: commands=', commands);\n\r\
      renderCommands();\n\r\
    }\n\r\
    function clickedDown(index) {\n\r\
      console.log('down: index=', index);\n\r\
      var tmp = commands.splice(index, 1)[0];\n\r\
      commands.splice(index + 1, 0, tmp);\n\r\
      console.log('down: commands=', commands);\n\r\
      renderCommands();\n\r\
    }\n\r\
    function clickedDelete(index) {\n\r\
      commands.splice(index, 1)[0];\n\r\
      renderCommands();\n\r\
    }\n\r\
    function doGET() {\n\r\
      let list = [];\n\r\
      commands.forEach(c => {\n\r\
        list.push(c[0]);\n\r\
      });\n\r\
      console.log('list: ', list);\n\r\
      var xhr = new XMLHttpRequest();\n\r\
      xhr.open('GET', './c?=' + list.join(',') + ',');\n\r\
      xhr.setRequestHeader('Content-Type', 'text/plain');\n\r\
      xhr.onload = function() {\n\r\
        if (xhr.status === 200) {\n\r\
          alert('Saved successfully :)');\n\r\
        } else {\n\r\
          alert('ERROR: Failed to save!');\n\r\
        }\n\r\
      };\n\r\
      xhr.send();\n\r\
    }\n\r\
    renderSelectOptions();\n\r\
    renderCommands();</script>\n\r"

#endif Config_HTML_h