var Module = {
    preRun: function() {
      function stdout(asciiCode) {
        document.getElementById("compliance_report").innerHTML += String.fromCharCode(asciiCode);
      }
  
      function stderr(asciiCode) {
        document.getElementById("compliance_tool_errors").innerHTML += String.fromCharCode(asciiCode);
      }
  
      FS.init(null, stdout, stderr);
    }
  };
