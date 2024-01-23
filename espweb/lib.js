function openTab(evt, tabName) {
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }
  tablinks = document.getElementsByClassName("tablinks");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }
  document.getElementById(tabName).style.display = "block";
  evt.currentTarget.className += " active";
}

function minmax(value, min, max) {
  if(parseInt(value) < min || isNaN(parseInt(value))) 
    return min; 
  else if(parseInt(value) > max) 
    return max; 
  else return parseInt(value);
}
function get(id, prop = 'value'){
  return document.getElementById(id)[prop];
}
//md5 lib
function t(n,t){
  var r=(65535&n)+(65535&t),e=(n>>16)+(t>>16)+(r>>16);
  return e<<16|65535&r
}
function r(n,t){
  return n<<t|n>>>32-t
}
function e(n,e,o,u,c,f){
  return t(r(t(t(e,n),t(u,f)),c),o)
}
function o(n,t,r,o,u,c,f){
  return e(t&r|~t&o,n,t,u,c,f)
}
function u(n,t,r,o,u,c,f){
  return e(t&o|r&~o,n,t,u,c,f)
}
function c(n,t,r,o,u,c,f){
  return e(t^r^o,n,t,u,c,f)
}
function f(n,t,r,o,u,c,f){
  return e(r^(t|~o),n,t,u,c,f)
}
function i(n,r){
  n[r>>5]|=128<<r%32,n[(r+64>>>9<<4)+14]=r;
  var e,i,a,h,d,l=1732584193,g=-271733879,v=-1732584194,m=271733878;
  for(e=0;e<n.length;e+=16)
    i=l,a=g,h=v,d=m,l=o(l,g,v,m,n[e],7,-680876936),
    m=o(m,l,g,v,n[e+1],12,-389564586),v=o(v,m,l,g,n[e+2],17,606105819),
    g=o(g,v,m,l,n[e+3],22,-1044525330),l=o(l,g,v,m,n[e+4],7,-176418897),
    m=o(m,l,g,v,n[e+5],12,1200080426),v=o(v,m,l,g,n[e+6],17,-1473231341),
    g=o(g,v,m,l,n[e+7],22,-45705983),l=o(l,g,v,m,n[e+8],7,1770035416),
    m=o(m,l,g,v,n[e+9],12,-1958414417),v=o(v,m,l,g,n[e+10],17,-42063),
    g=o(g,v,m,l,n[e+11],22,-1990404162),l=o(l,g,v,m,n[e+12],7,1804603682),
    m=o(m,l,g,v,n[e+13],12,-40341101),v=o(v,m,l,g,n[e+14],17,-1502002290),
    g=o(g,v,m,l,n[e+15],22,1236535329),l=u(l,g,v,m,n[e+1],5,-165796510),
    m=u(m,l,g,v,n[e+6],9,-1069501632),v=u(v,m,l,g,n[e+11],14,643717713),
    g=u(g,v,m,l,n[e],20,-373897302),l=u(l,g,v,m,n[e+5],5,-701558691),
    m=u(m,l,g,v,n[e+10],9,38016083),v=u(v,m,l,g,n[e+15],14,-660478335),
    g=u(g,v,m,l,n[e+4],20,-405537848),l=u(l,g,v,m,n[e+9],5,568446438),
    m=u(m,l,g,v,n[e+14],9,-1019803690),v=u(v,m,l,g,n[e+3],14,-187363961),
    g=u(g,v,m,l,n[e+8],20,1163531501),l=u(l,g,v,m,n[e+13],5,-1444681467),
    m=u(m,l,g,v,n[e+2],9,-51403784),v=u(v,m,l,g,n[e+7],14,1735328473),
    g=u(g,v,m,l,n[e+12],20,-1926607734),l=c(l,g,v,m,n[e+5],4,-378558),
    m=c(m,l,g,v,n[e+8],11,-2022574463),v=c(v,m,l,g,n[e+11],16,1839030562),
    g=c(g,v,m,l,n[e+14],23,-35309556),l=c(l,g,v,m,n[e+1],4,-1530992060),
    m=c(m,l,g,v,n[e+4],11,1272893353),v=c(v,m,l,g,n[e+7],16,-155497632),
    g=c(g,v,m,l,n[e+10],23,-1094730640),l=c(l,g,v,m,n[e+13],4,681279174),
    m=c(m,l,g,v,n[e],11,-358537222),v=c(v,m,l,g,n[e+3],16,-722521979),
    g=c(g,v,m,l,n[e+6],23,76029189),l=c(l,g,v,m,n[e+9],4,-640364487),
    m=c(m,l,g,v,n[e+12],11,-421815835),v=c(v,m,l,g,n[e+15],16,530742520),
    g=c(g,v,m,l,n[e+2],23,-995338651),l=f(l,g,v,m,n[e],6,-198630844),
    m=f(m,l,g,v,n[e+7],10,1126891415),v=f(v,m,l,g,n[e+14],15,-1416354905),
    g=f(g,v,m,l,n[e+5],21,-57434055),l=f(l,g,v,m,n[e+12],6,1700485571),
    m=f(m,l,g,v,n[e+3],10,-1894986606),v=f(v,m,l,g,n[e+10],15,-1051523),
    g=f(g,v,m,l,n[e+1],21,-2054922799),l=f(l,g,v,m,n[e+8],6,1873313359),
    m=f(m,l,g,v,n[e+15],10,-30611744),v=f(v,m,l,g,n[e+6],15,-1560198380),
    g=f(g,v,m,l,n[e+13],21,1309151649),l=f(l,g,v,m,n[e+4],6,-145523070),
    m=f(m,l,g,v,n[e+11],10,-1120210379),v=f(v,m,l,g,n[e+2],15,718787259),
    g=f(g,v,m,l,n[e+9],21,-343485551),l=t(l,i),g=t(g,a),v=t(v,h),m=t(m,d);
  return[l,g,v,m]
}
function a(n){
  var t,r="";
  for(t=0;t<32*n.length;t+=8)
    r+=String.fromCharCode(n[t>>5]>>>t%32&255);
  return r
}
function h(n){
  var t,r=[];
  for(r[(n.length>>2)-1]=void 0,t=0;t<r.length;t+=1)
  r[t]=0;
  for(t=0;t<8*n.length;t+=8)
  r[t>>5]|=(255&n.charCodeAt(t/8))<<t%32;
  return r
}
function d(n){
  return a(i(h(n),8*n.length))
}
function l(n,t){
  var r,e,o=h(n),u=[],c=[];
  for(u[15]=c[15]=void 0,o.length>16&&(o=i(o,8*n.length)),r=0;16>r;r+=1)
  u[r]=909522486^o[r],c[r]=1549556828^o[r];
  return e=i(u.concat(h(t)),512+8*t.length),a(i(c.concat(e),640))
}
function g(n){
  var t,r,e="0123456789abcdef",o="";
  for(r=0;r<n.length;r+=1)
    t=n.charCodeAt(r),o+=e.charAt(t>>>4&15)+e.charAt(15&t);
  return o
}
function v(n){
  return unescape(encodeURIComponent(n))
}
function m(n){
  return d(v(n))
}
function md5(n){
  return g(m(n))
}

function bytestostr(bytes){
  let res = '';
  for (let i = 0; i < bytes.length; i++){
    res += String.fromCharCode(bytes[i]);
  }
  return res;
}

function bytestohex(bytes, sepr = ''){
  let res = '';
  for (let i = 0; i < bytes.length; i++){
    res += ('0' + bytes[i].toString(16)).slice(-2) + sepr;
  }
  return res.slice(0, res.length - sepr.length);
}

function posinstr(data, pattern, start = 0){
  let temp = data.substr(start).search(pattern);
  return ((temp < 0)? temp:(temp+start));
}

function intfrombytes(intbytes, edian = 'little', signed = false){
  let count = intbytes.length - 1;
  let res = 0;
  let neg = (signed && (((edian == 'little') && (intbytes[count] > 127)) || ((edian == 'big') && (intbytes[0] > 127))));
  if (neg) for (let i = 0; i <= (count); i++) intbytes[i] = 255 - intbytes[i];
  if (edian == 'big'){
    for (let i = 0; i < (count); i++){
      res += intbytes[i];
      res *= 256;
    }
    res += intbytes[count];
  }else if (edian == 'little'){
    for (let i = count; i > 0; i--){
      res += intbytes[i];
      res *= 256;
    }
    res += intbytes[0];
  }
  if (neg) res = -1 - res;
  return res;
}

function DateTime(UNIX_timestamp){
  var a = new Date(UNIX_timestamp * 1000);
  var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
  var year = a.getFullYear().toString();
  var month = (a.getMonth() + 1).toString();
  var date = a.getDate().toString();
  var hour = ('0' + a.getHours().toString()).slice(-2);
  var min = ('0' + a.getMinutes().toString()).slice(-2);
  var sec = ('0' + a.getSeconds().toString()).slice(-2);
  var time = date + '.' + month + '.' + year + ' ' + hour + ':' + min + ':' + sec ;
  return time;
}

var x_25 = {
  init: 0xFFFF,
  invertedInit: 0xFFFF,
  xorOut: 0xFFFF,
  refOut: true,
  refIn: true,
  table: Uint16Array.from([
     0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD,
     0x6536, 0x74BF, 0x8C48, 0x9DC1, 0xAF5A, 0xBED3,
     0xCA6C, 0xDBE5, 0xE97E, 0xF8F7, 0x1081, 0x0108,
     0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
     0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64,
     0xF9FF, 0xE876, 0x2102, 0x308B, 0x0210, 0x1399,
     0x6726, 0x76AF, 0x4434, 0x55BD, 0xAD4A, 0xBCC3,
     0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
     0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E,
     0x54B5, 0x453C, 0xBDCB, 0xAC42, 0x9ED9, 0x8F50,
     0xFBEF, 0xEA66, 0xD8FD, 0xC974, 0x4204, 0x538D,
     0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
     0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1,
     0xAB7A, 0xBAF3, 0x5285, 0x430C, 0x7197, 0x601E,
     0x14A1, 0x0528, 0x37B3, 0x263A, 0xDECD, 0xCF44,
     0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
     0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB,
     0x0630, 0x17B9, 0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5,
     0xA96A, 0xB8E3, 0x8A78, 0x9BF1, 0x7387, 0x620E,
     0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
     0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862,
     0x9AF9, 0x8B70, 0x8408, 0x9581, 0xA71A, 0xB693,
     0xC22C, 0xD3A5, 0xE13E, 0xF0B7, 0x0840, 0x19C9,
     0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
     0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324,
     0xF1BF, 0xE036, 0x18C1, 0x0948, 0x3BD3, 0x2A5A,
     0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E, 0xA50A, 0xB483,
     0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
     0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF,
     0x4C74, 0x5DFD, 0xB58B, 0xA402, 0x9699, 0x8710,
     0xF3AF, 0xE226, 0xD0BD, 0xC134, 0x39C3, 0x284A,
     0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
     0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1,
     0xA33A, 0xB2B3, 0x4A44, 0x5BCD, 0x6956, 0x78DF,
     0x0C60, 0x1DE9, 0x2F72, 0x3EFB, 0xD68D, 0xC704,
     0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
     0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68,
     0x3FF3, 0x2E7A, 0xE70E, 0xF687, 0xC41C, 0xD595,
     0xA12A, 0xB0A3, 0x8238, 0x93B1, 0x6B46, 0x7ACF,
     0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
     0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022,
     0x92B9, 0x8330, 0x7BC7, 0x6A4E, 0x58D5, 0x495C,
     0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
  ])
};

function crc16X25 (data, seed = false) {

  const {
     init,
     invertedInit,
     xorOut,
     refOut,
     refIn,
     table
  } = x_25;

  let crc = seed ? seed ^ xorOut : (refIn ? invertedInit || 0 : init);
  
  if (refOut) {
     for (const b of data)
        crc = table[(b ^ crc) & 0xFF] ^ (crc >> 8 & 0xFF);
  }
  else {
     for (const b of data)
        crc = table[((crc >> 8) ^ b) & 0xFF] ^ (crc << 8);
  }

  return (crc ^ xorOut) & 0xFFFF;
}
