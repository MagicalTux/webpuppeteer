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

function web21_wait() {
	tab.waitFinish();
	while(true) {
		if (tab.eval("LoadChk()")) return;
		sys.sleep(100);
	}
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
tab.wait(3);
tab.waitFinish();

sys.log("Waiting for menu to appear to click tori");

// go in tori mode
while(true) {
	web21_wait();
	var t = tab.document().frameDocument("M2").find({tagname:"img", src:"/web21/img/ACAH9K000_tori.gif"});
	if (t.length == 0) continue;
	// give time for page to settle
	t[0].click();
	tab.wait();
	tab.waitFinish();
	break;
}

function waitClick(img_src) {
	sys.sleep(300);
	while(true) {
		web21_wait();
		if (!tab.document().frameDocument("ot3").frameDocument("C2")) continue; // loading
		var t = tab.document().frameDocument("ot3").frameDocument("C2").find({tagname:"img", src:img_src});
		if (t.length == 0) continue;
		t[0].click();
		sys.sleep(500);
		web21_wait();
		break;
	}
}

sys.log("Clicking nyuusyu");
waitClick("/web21/img/ACAH9K000_nyuusyu.gif");
sys.log("Clicking domi");
waitClick("/web21/img/ACAH9K000_domi.gif");
sys.log("Clicking download");
waitClick("/web21/img/ACAH9K000_download.gif");
// choose right type
tab.document().frameDocument("ot3").frameDocument("C2").getElementsByName("SelectValue")[0].eval("this.value=2"); // CSV
// click download
sys.log("Clicking download");
waitClick("/web21/img/ACAH9K000_download.gif");

sys.log("Downloading...");
while(true) {
	var file = tab.getDownloadedFile();
	if (!file) {
		sys.sleep(100);
		continue;
	}
	sys.log("Got file: "+file.filename);
	sys.filePutContentsB64("/home/magicaltux/Downloads/"+file.filename, file.filedata);
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/smbc_jpy_push", "data="+encodeURIComponent(file.filedata), api_key, api_secret);
	sys.log(res);
	break;
}

sys.log("clicking logout");
tab.document().frameDocument("H0").find({tagname:"img", src:"/web21/img/ACAH9K000_logout_n.gif"})[0].click();
sys.sleep(500);
tab.waitFinish();

