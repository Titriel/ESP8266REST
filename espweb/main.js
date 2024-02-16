const xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {handelres(this);}
document.getElementById("linfo").innerHTML = "Config:";
document.getElementById("info").value = config;
var jconfig = JSON.parse(config);
const ts = new Date();
var from = ""
var SysHash = "";
var Auth = "";
var jresponce;
var EvSo = false;
var source = false;
document.getElementById("firstTab").click();
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
    if (jresponce['msg'] == "From is peregistert."){
      from = jresponce['UUID'].substring(0, 8);
      document.getElementById("RqSession").innerHTML="Request Session";
    }
    if ((jresponce['msg'] == "Session deleted.") || (call.status == 401)){
      from = "";
      document.getElementById("RqSession").innerHTML="Get Haeder: From";
    }    
    Auth = md5(jresponce.UUID + SysHash);
    /*console.log(jresponce.UUID);
    console.log(call.getResponseHeader("Content-Type"));
    if (call.getAllResponseHeaders().indexOf("X-Token") >= 0) {
      console.log(call.getResponseHeader("X-Token"));
    }
    console.log(call.status);*/
  }
}
function dorequest(path = "/", methode = "GET", content = "{}") {
  let ip = document.getElementById("IP").value;
  xhttp.open(methode, "http://"+ip+path);
  xhttp.setRequestHeader("From",from);
  let utc = Math.floor((new Date()).getTime() / 1000).toString();
  xhttp.setRequestHeader("X-UTC",utc);
  let reqtext = "URL: http://" + ip+path + "\r\n";
  reqtext += "Methode: " + methode + "\r\n";
  reqtext += "Haeder From: " + from + "\r\n";
  reqtext += "Haeder X-UTC: " + utc + "\r\n";
  if ((path == "/") && (methode == "GET")){
    xhttp.setRequestHeader("X-Auth", md5(SysHash+from));
    reqtext += "Haeder X-Auth: " + md5(SysHash+from) + "\r\n";
  }else{
    xhttp.setRequestHeader("X-Auth",Auth);
    reqtext += "Haeder X-Auth: " + Auth + "\r\n";
  }
  if ((methode == "POST") || (methode == "PUT") || (methode == "PATCH")){
    xhttp.setRequestHeader("Content-Type","application/json");
    xhttp.send(content.replaceAll('``','"'));
    reqtext += JSON.stringify(JSON.parse(content.replaceAll('``','"')),undefined, 2);

  }else{
    xhttp.send();
  }
  document.getElementById("req").value = reqtext;
  document.getElementById("linfo").innerHTML = "ESP8266-Response:";
  document.getElementById("info").value = "Abfrage gestartet.";
}
function startSSE(user, pass){
  console.log(window.EventSource);
  dorequest('/','PUT','{}');
  //dorequest('/','PUT','{"user": "'+user+'", "pass": "'+pass+'"}');
  if (!EvSo) {
    setTimeout(aktSSE, 500);
  }
}
function aktSSE(){
  let ip = document.getElementById("IP").value;
  if ((typeof(source.readyState) !== "undefined") && source.readyState === EventSource.OPEN){
    source.close();
  }
  source = new EventSource("http://"+ip+'/events', {
    withCredentials: false
  });
  source.addEventListener('open', function(e) {
    document.getElementById("sse").value += "Events Connected\r\n";
  }, false);

  source.addEventListener('close', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      document.getElementById("sse").value += "Events Closed\r\n";
    }
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      document.getElementById("sse").value += "Events Disconnected\r\n";
      e.target.close();
      //setTimeout(aktSSE, 15000);
    }
  }, false);

  source.addEventListener('message', function(e) {
    document.getElementById("sse").value += "message["+e.lastEventId+"]: "+e.data+"\r\n";
  }, false);

  source.addEventListener('os', function(e) {
    document.getElementById("sse").value += "os["+e.lastEventId+"]: "+e.data+"\r\n";
  }, false);

  source.addEventListener('Serial0', function(e) {
    let ser = JSON.parse(e.data);
    if (typeof(ser.RxT) !== "undefined") {
      document.getElementById("Serialres").value += atob(ser.RxT);
    }
    document.getElementById("sse").value += "Serial0["+e.lastEventId+"]: "+e.data+"\r\n";
  }, false);
}

function loadconfig() {
  for (const [key, value] of Object.entries(jconfig)) {
    if(document.getElementById("IP").value == ""){
      document.getElementById("IP").value = value.ip;
      document.getElementById("Pass").value = value.pass;
      SysHash =md5(value.pass);
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
  document.getElementById("RqSession").innerHTML="Get Haeder: From";
}
function changeedpass() {
  SysHash =md5(document.getElementById("Pass").value);
}