var Module = {
  'print': function(text) {
    document.getElementById("compliance_report").innerHTML += text + "<br/>";
  },
  'printErr': function(text) { document.getElementById("compliance_tool_errors").innerHTML += text + "<br/>"; }
};
