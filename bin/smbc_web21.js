sys.include("config.js");
// from https://e-biz.smbc.co.jp/core/exec/servlet/ACH99OMCL_SKBCNTL?action=action3&refreshRandomNum=862&UkeID=ACHE9H001

function web21_encode_password(table, password) {
	var retStr = "";
	var t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	var randomNum = table[table.length-1];
	var table1Pos = parseInt(randomNum.substring(0,1));
	var table2Pos = parseInt(randomNum.substring(1,2));
	var table3Pos = parseInt(randomNum.substring(2,3));

	for(i = 0; i < password.length; i++) {
		var temp = password.substring(i,i+1);
		index = t.indexOf(temp);
		temp= table[62 * table1Pos + index];
		index = t.indexOf(temp);
		temp= table[62 * table2Pos + index];
		index = t.indexOf(temp);
		temp= table[62 * table3Pos + index];
		retStr = retStr + temp;
	}
	return retStr;
}

function web21_picktable(table) {
	var picktmp = "";
	for(i = 0; i < 10; i++){
		picktmp= picktmp + table[62 * i + 0];
		picktmp= picktmp + table[62 * i + 30];
		picktmp= picktmp + table[62 * i + 61];
	}
	return picktmp;
}

var tab = sys.newTab();
tab.overrideUserAgent("Mozilla/5.0 (Windows; U; MSIE 9.0; Windows NT 9.0; en-US)"); // make it look like MSIE9
tab.setDefaultCharset("SJIS");
tab.load("https://e-biz.smbc.co.jp/core/exec/servlet/ACH99OMCL_SKBCNTL?action=action3&refreshRandomNum=862&UkeID=ACHE9H001");
var table = tab.document().textContent();
table = table.substring(table.indexOf("RET_VALUE=") + 10).split(",");

tab.post("https://e-biz.smbc.co.jp/core/exec/servlet/ACH99OMCL_WEBCNTL", "User="+valuedoor_id+"&Pwd1="+web21_encode_password(table, valuedoor_password)+"&hid1=&UkeID=ACHE9H001&APNextScrID=ACH999002&pickTable="+web21_picktable(table)+"&_W_SoftKeyBoardFlag=1");

tab.document().findAllContaining("Web21")[0].click();
tab.wait();

tab.interact();

