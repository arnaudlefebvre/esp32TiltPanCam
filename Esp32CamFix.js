function fixPort() { var dirtysrc = document.getElementById('stream').getAttribute('src');
var src = dirtysrc.substring(8);
var dirtyPort = src.substring(src.indexOf(':')+1,src.lastIndexOf('/'));
if (dirtyPort.indexOf(':') != -1) {
    var port = Number(dirtyPort.substring(0,dirtyPort.indexOf(':')));
    dirtysrc = dirtysrc.replace(dirtyPort,port+1);
    document.getElementById('stream').setAttribute('src',dirtysrc);
}}
function rotateStream() { var dirtysrc=document.getElementById('stream').style.transform='rotate(-90deg)'; }
//rotateStream();
//fixPort();

document.addEventListener('keyup', logKey);

function logKey(e) {
  var evt = new Event('change');
  if(`${e.code}` == 'KeyW') {
	  var elt = document.getElementById('CamTilt');
	  var val = Number(elt.value) + 1;
	  //alert(val);
	  elt.value = val;
	  elt.dispatchEvent(evt);
  } else if (`${e.code}` == 'KeyS') {
	  var elt = document.getElementById('CamTilt');
	  var val = Number(elt.value) - 1;
	  //alert(val);
	  elt.value = val;
	  elt.dispatchEvent(evt);
  } else if (`${e.code}` == 'KeyA') {
	  var elt = document.getElementById('CamRotate');
	  var val = Number(elt.value) + 1;
	  //alert(val);
	  elt.value = val;
	  elt.dispatchEvent(evt);
  }else if (`${e.code}` == 'KeyD') {
	  var elt = document.getElementById('CamRotate');
	  var val = Number(elt.value) - 1;
	  //alert(val);
	  elt.value = val;
	  elt.dispatchEvent(evt);
  }  
}