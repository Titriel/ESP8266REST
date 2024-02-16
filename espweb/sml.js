var testdata = atob('GxsbGwEBAQF2C0VTWU0adxivixdiAGIAcmMBAXYBBEVTWQhFU1mEpIsXCwkBRVNZEQOhGncBAWPOigB2C0VTWU0adxivixhiAGIAcmMHAXcBCwkBRVNZEQOhGncIAQBiCv//AHJiAWUIOoSkfHcHgYHHggP/AQEBAQRFU1kBdwcBAAAACf8BAQEBCwkBRVNZEQOhGncBdwcBAAEIAP9kAAKAAWIeUvxZAAAAIBcOA7YBdwcBAAEIAf8BAWIeUvxZAAAAAAAgal8BdwcBAAEIAv8BAWIeUvxZAAAAIBbtmVYBdwcBABAHAP8BAWIbUv5ZAAAAAAAALzEBdwcBACQHAP8BAWIbUv5ZAAAAAAAAAnkBdwcBADgHAP8BAWIbUv5ZAAAAAAAAG0EBdwcBAEwHAP8BAWIbUv5ZAAAAAAAAEXcBdwcBACAHAP8BAWIjUv9jCTgBdwcBADQHAP8BAWIjUv9jCTEBdwcBAEgHAP8BAWIjUv9jCTsBAQFjKosAdgtFU1lNGncYr4sZYgBiAHJjAgFxAWNtigAAGxsbGxoB/9s=');
var SMLtable= {
  'lastbin': null,
  'lasthex': '',
  'timeoffset': {
    '09014553591103a11a77': 1526571812
  },
  'tag': {
    'Escape': '1b1b1b1b',
    'V1Start': '01010101',
    'End': '00',
    'Close': '1a'
  },
  'commands': {
    '101': 'getOpenResponse',
    '201': 'getCloseResponse',    
    '701': 'getListResponse'
  },  
  'unit': {
    '1b': 'W',
    '1e': 'Wh',
    '23': 'V'
  },
  'mainframe':[
    'transactionId',
    'groupNo',
    'abortOnError',
    ['msg', -1, 0],
    'crc16',
    'end'
  ],
  'OBIS': {
    '8181c78203ff': 'Manufactor',
    '8181c78205ff': 'Public_Key',
     '100000009ff': 'Server_ID',
     '100010800ff': 'Total_Work',
     '100010801ff': 'Work_T1',
     '100010802ff': 'Work_T2',
     '100020800ff': 'Total_Work_Inj',
     '100020801ff': 'Work_T1_Inj',
     '100020802ff': 'Work_T2_Inj',
     '100100700ff': 'Total_Power',
     '100240700ff': 'Power_L1',
     '100380700ff': 'Power_L2',
     '1004c0700ff': 'Power_L3',
     '100200700ff': 'Voltage_L1',
     '100340700ff': 'Voltage_L2',
     '100480700ff': 'Voltage_L3'
  },
  'msg': {
    '101': [
      'codepage',
      'clientId',
      'reqFileId',
      'serverID',
      'refTime',
      'smlVersion'
    ],
    '201':[
      'globalSignature'
    ],
    '701':[
      'clientId',
      'serverID',
      'ListName',
      ['actSensorTime', 1, 0],
      ['OBISlist', 1, 0],
      'listSignature',
      'actGatewayTime'
    ]
  },
  'list': {
    'actSensorTime': [
      'choice',
      'secIndex'
    ],
    'OBISlist': [
      ['OBIS',0 , 0]
    ],    
    'OBIS':[
      'OBISkey',
      'status',
      'valTime',
      'unit',
      'scaler',
      'value',
      'valueSignature',
    ]   
  }
}

var debug;
function smlcheck(smldata){
  let datalength = smldata.length;
  let hexdata = SMLtable.lasthex;
  let olddatalength = hexdata.length/2;
  let data = new Uint8Array(datalength + olddatalength);
  let i;
  for (i = 0; i < olddatalength; i++){
    data[i] = SMLtable.lastbin[i];
  }
  for (let j = 0; j < datalength; j++){
    data[i+j] = smldata.charCodeAt(j);
    hexdata += ('0' + (data[i+j] & 0xFF).toString(16)).slice(-2);
  }

  let t = SMLtable.tag;
  let res = [];
  let End = 0;
  let Start = 0;
  while (End !== 13){
    Start = posinstr(hexdata, t.Escape + t.V1Start.substr(0,2), End);
    End = posinstr(hexdata, t.End + t.Escape + t.Close, Start) + 14;
    if ((Start < End) && (Start > -1)){
      let ixEnd = End/2;
      let crccalc = crc16X25(data.slice(Start/2, ixEnd));
      let crc = data[ixEnd] + data[ixEnd + 1]*256;
      res.push({'Start': Start, 'CRC_Ok': (crc===crccalc)});
    }
  }
  SMLtable.lastbin = data;
  SMLtable.lasthex = hexdata;
  return res;
}
function smltryFindOBISkeys(c){
  let data = SMLtable.lastbin;
  let hexdata = SMLtable.lasthex;
  let hexc = c*2;  
  while (true){
    hexc = posinstr(hexdata, '7707', hexc)
    if (hexc === -1) return;
    c = hexc / 2
    if (hexdata.substr(hexc + 14, 2) === 'ff'){
      let f = SMLtable.list.OBIS;
      OBIS = intfrombytes(data.slice(c + 2, c + 8), 'big').toString(16);
      if (OBIS in SMLtable.OBIS){
        console.log('OBIS Kennzahl: ', OBIS, SMLtable.OBIS[OBIS], ' -> Key: ');
      }else{
        console.log('*** Die OBIS Kennzahl ist unbekannt ***', OBIS, ' -> Key: ');
      }
      c++;
      let res = {};
      for (let i = 0; i < 7; i++){
        let decdata = smldecodedata(c);
        if (!decdata.Ok) return decdata;
        if (i == 0){
          console.log (decdata.value);
        }else{
          res[f[i]] = decdata.value;
        }
        c = decdata.c;
      }
      hexc = c*2;
      console.log(res);
      if (res.scaler === null){
        console.log(res.value);
      }else{
        if (res.unit.toString(16) in SMLtable.unit){
          unit = SMLtable.unit[res.unit.toString(16)];
        }else{
          unit = '[unbekannte Einheit]';
        }
        temp = res.value.toString();
        if (res.scaler < 0){
          temp = temp.slice(0, temp.length + res.scaler) + ',' + temp.slice(res.scaler);
        }else{
          temp += '0'.repeat(res.scaler);
        }
        console.log(temp + ' ' + unit);
      }
    }
  }
}

function getTelegramm(c, decode = true){
  let res = {'Ok': true, 'c': c, 'decoded':{}};
  let t = SMLtable.tag;
  let data = SMLtable.lastbin;
  let hexdata = SMLtable.lasthex;
  let hexc = c*2;  
  if (hexdata.substr(hexc, 8) == t.Escape){
    res.Version = (hexdata.substr(hexc + 8, 8) === t.V1Start)?'V1':'V2';
    c += 8;
    while (smldformat(c).h == 7){
      let msg = smlgetMsg(c);
      if (!msg.Ok){
        res.failed = msg;
        res.Ok = false;
        break;
      }
      if (crc16X25(data.slice(c, msg.c - 4)) !== msg.data.crc16){
        res.failed = {'Ok': false, 'error': "Cheksummenfehler in der Nachricht", 'CRC': crc16X25(data.slice(c, msg.c - 4))};
        res.Ok = false;
      }; 
      res[bytestohex(msg.data.transactionId)] = msg.data;
      if (('getListResponse' in msg.data) && ('OBISlist' in msg.data.getListResponse)){
        if (decode) res.decoded = smlcalcflat(msg.data.getListResponse.OBISlist, 'OBISlist');
        res.decoded.actSensorTime = smlcalcflat(msg.data.getListResponse, 'getListResponse').actSensorTime;
      }
      c = msg.c;
    }
    res.c = c;
  }
  return res;
}

function smlgetMsg(c, d = 0, obj = []){
  let res = {'Ok': true, 'data': {}, 'flat': [], 'c': c};
  let f;
  if (d === 0){
    f = SMLtable.mainframe;
  }else if (obj[1] == -1){
    f = SMLtable[obj[0]][obj[2]];
  }else{
    f = SMLtable.list[obj[0]];
  }

  let idf = smldformat(c);
  let decdata, block, names;
  c = idf.c;
  if (idf.h == 7){
    let j = 0;
    for (let i = 0; i < idf.l; i++){
      if (typeof f[i - j] === 'string'){
        decdata = smldecodedata(c, f[i - j]);
        if (!decdata.Ok) return  decdata;
        res.data[f[i - j]] = decdata.value;
        c = decdata.c;
      }else{
        if (f[i - j][1] === -1){
          decdata = smldecodedata(c + 1);
          if (!decdata.Ok) return decdata;
          f[i - j][2] = decdata.value.toString(16);
          names = SMLtable.commands[decdata.value.toString(16)];
          block = smlgetMsg(decdata.c, d + 1, f[i - j]);
          if (!block.Ok) return block;
          res.data[names] = block.data;
          c = block.c;
        }else if (f[i - j][1] === 0){
          decdata = smldecodedata(c + 1)
          if (!decdata.Ok) return  decdata;
          names = f[i - j][0];
          let deckey = intfrombytes(decdata.value, 'big').toString(16);
          if (deckey in SMLtable[names]){          
            block = smlgetMsg(c, d + 1, f[i - j])
            if (!block.Ok) return block;
            res.data[SMLtable[names][deckey]] = block.data;
            c = block.c;
            j++;
          }else{
            console.log('unknowen key', deckey, names);
            i--;
            j--;
          }
        }else{
          block = smlgetMsg(c, d + 1, f[i]);
          if (!block.Ok) return block;
          names = f[i - j][0];
          res.data[f[i - j][0]] = block.data;
          c = block.c;
        }
      }
    }
  }else{
    console.log(res);
    res = {'Ok': false, 'error': "Unerwartete Blockstruktur"}
  }
  res.c = c;
    //res['flat'] = smlcalcflat(res['data'], names);
  return res
}

function smldformat (c){
  let data = SMLtable.lastbin;
  let res ={'h': Math.floor(data[c] / 16) & 7, 'l': 0};
  do{
    res.l = res.l * 16 + (data[c] & 15);
  }while (data[c++] > 127);
  res.c = c;
  return res;
}

function smldecodedata(c, names = ''){
  let data = SMLtable.lastbin;
  let idf = smldformat(c);
  let res = {'Ok': false, 'c': idf.c + idf.l - 1};
  let sign = false;
  switch(idf.h) {
    case 0:
      if (idf.l === 0){
        res.value = true;
        res.c++;
      }else if (idf.l === 1){
        res.value = null;
      }else{
        res.value = data.slice(idf.c, res.c);
      }
    break;      
    case 5: sign = true;
    case 6:
      if (names == 'crc16'){
        res.value = intfrombytes(data.slice(idf.c, res.c));
      }else{
        res.value = intfrombytes(data.slice(idf.c, res.c), 'big', sign);
      }
    break;   
    default:
      res.error = 'Unerwarteter Datentyp';
  }
  res.Ok = ('value' in res);
  return res;
}

function smlcalcflat(data, names){
  let res = {};
  if (names === 'OBISlist'){
  let byteman;
    for (let key in data){
      if (key == 'Manufactor'){
        byteman = data[key].value;
        res[key] = bytestostr(data[key].value);
      }else if (key == 'Server_ID'){
        if (res.Manufactor == 'ESY'){
          let srvhex = bytestohex(data[key].value, ' ');
          let b = data[key].value;
          temp = intfrombytes(b.slice(6), 'big').toString();
          temp = ' ' + temp.slice(0, 4) + ' ' + temp.slice(4);
          temp = srvhex.slice(3, 6) + bytestostr(b.slice(2, 5)) + srvhex.slice(14, 17) + temp;
          res.Key = temp;
        }
        res[key] = bytestohex(data[key].value, ' ');
      }else{
        let d = data[key];
        //temp = str(d['value'] * pow(10, d['scaler'])) + ' ' + SMLtable['unit'][d['unit']]
        let temp = d.value.toString();
        if (d.scaler < 0){
          temp = temp.slice(0, temp.length + d.scaler) + ',' + temp.slice(d.scaler);
        }else{
          temp += '0'.repeat(d.scaler);
        }
        res[key] = temp + ' ' + SMLtable.unit[d.unit.toString(16)];
      }
    }
  }else if (names === 'getListResponse'){
    let serverID = bytestohex(data.serverID);
    if (serverID in SMLtable.timeoffset){
      if (SMLtable.timeoffset[serverID] > 0){
        res.actSensorTime = DateTime(data.actSensorTime.secIndex + SMLtable.timeoffset[serverID]);
      }else{
        let now = Math.floor(Date.now()/1000);
        SMLtable.timeoffset[serverID] = now - data.actSensorTime.secIndex;
        res.actSensorTime = DateTime(now);
      }
    }else{
      res.actSensorTime = data.actSensorTime.secIndex;
    }
  }
  return res
}