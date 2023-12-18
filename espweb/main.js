const xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {handelres(this);}
document.getElementById("linfo").innerHTML = "Config:";
document.getElementById("info").value = config;
var jconfig = JSON.parse(config);
const ts = new Date();
var from = md5(ts.toString()).substring(0, 8);
var SysHash = "";
var Auth = "";
var jresponce;
loadconfig()

function handelres(call){
  if (call.readyState == 4) {
    let restext = "Statuscode: " + 
                  call.status +
                  "\r\nLänge: " +
                  call.responseText.length.toString() +
                  "\r\n" +
                  call.responseText;
    document.getElementById("info").value = restext;
    jresponce = JSON.parse(call.responseText);
    Auth = md5(jresponce.UUID + SysHash);
    console.log(jresponce.UUID);
    console.log(call.getResponseHeader("Content-Type"));
    if (call.getAllResponseHeaders().indexOf("X-Token") >= 0) {
      console.log(call.getResponseHeader("X-Token"));
    }
    console.log(call.status);
  }
}
function dorequest(path = "/", methode = "GET", content = "{}") {
  let ip = document.getElementById("IP").value;
  xhttp.open(methode, "http://"+ip+path);
  xhttp.setRequestHeader("From",from);
  if ((path == "/") && (methode == "GET")){
    xhttp.setRequestHeader("X-Auth", md5(SysHash+from));
  }else{
    xhttp.setRequestHeader("X-Auth",Auth);
  }
  if ((methode == "POST") || (methode == "PUT") || (methode == "PATCH")){
    xhttp.setRequestHeader("Content-Type","application/json");
    xhttp.send(content.replaceAll('``','"'));
  }else{
    xhttp.send();
  }
  document.getElementById("linfo").innerHTML = "ESP8266-Response:";
  document.getElementById("info").value = "Abfrage gestartet.";
}
function loadconfig() {
  for (const [key, value] of Object.entries(jconfig)) {
    if(document.getElementById("IP").value == ""){
      document.getElementById("IP").value = value.ip;
      document.getElementById("Pass").value = value.pass;
    }
    let opt = document.createElement('option');
    opt.value = key;
    opt.innerHTML = key;
    document.getElementById('Config').appendChild(opt);
  }
}
function changeconfig() {
  let confitem = document.getElementById("Config").value;
  document.getElementById("IP").value = jconfig[confitem].ip;
  document.getElementById("Pass").value = jconfig[confitem].pass;
  SysHash =md5(jconfig[confitem].pass)
}
function changeedpass() {
  SysHash =md5(document.getElementById("Pass").value);
}