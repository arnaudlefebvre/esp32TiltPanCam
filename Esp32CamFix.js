function fixPort() { var dirtysrc=document.getElementById('stream').getAttribute('src');var src=dirtysrc.substring(8);var dirtyPort=src.substring(src.indexOf(':')+1,src.lastIndexOf('/'));if(dirtyPort.indexOf(':')!=-1){var port=dirtyPort.substring(0,dirtyPort.indexOf(':'))+"1";dirtysrc=dirtysrc.replace(dirtyPort,port);document.getElementById('stream').setAttribute('src',dirtysrc);}}
function rotateStream() { var dirtysrc=document.getElementById('stream').style.transform='rotate(-90deg)'; }
rotateStream();
fixPort();