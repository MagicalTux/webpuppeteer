// JapanNet Bank - BA Plus
sys.include("config.js");

var tab = sys.newTab();
tab.load("https://login.japannetbank.co.jp/balogin_L.html");

sys.log("JapanNet: Logging in...");

tab.document().getElementsByName("CifNo")[0].setAttribute("value", jpnet_account);
tab.document().getElementsByName("UserId")[0].setAttribute("value", jpnet_login);
tab.document().getElementsByName("Pw")[0].setAttribute("value", jpnet_password);

tab.document().getElementsByName("login")[0].click();
tab.waitFinish(500);

if (tab.document().getElementsByName("Pw")[0]) {
	sys.log("JapanNet: Did not properly logout, re-login required");
	tab.document().getElementsByName("CifNo")[0].setAttribute("value", jpnet_account);
	tab.document().getElementsByName("UserId")[0].setAttribute("value", jpnet_login);
	tab.document().getElementsByName("Pw")[0].setAttribute("value", jpnet_password);
	tab.document().findFirst("input[type=image]").click();
	tab.waitFinish(500);
}

try {
	sys.log("JapanNet: Getting history...");
	tab.document().eval("toOthers()"); // account activity/balance/etc
	tab.waitFinish(500);
	tab.document().eval("mySubmit('202')"); // access movements list
	tab.waitFinish(500);
	tab.document().findFirst("input[type=button]").click();
	tab.waitFinish(500);
	tab.document().findFirst("form").eval("ToHonjitsu(this)");
	tab.waitFinish(500);
	tab.interact();
	sys.log("JapanNet: Downloading...");
	tab.document().eval("CsvClick()"); // access movements list
	tab.waitFinish(500);

	var win = tab.getNewWindow();
	if (!win) throw "Got no window while expecting one";

	while(true) {
		var file = win.getDownloadedFile();
		if (!file) {
			sys.sleep(100);
			continue;
		}
		break;
	}

	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/push_csv", "bank=38e619ac-7b9a-49a8-8e70-3628c980d1f0&charset=SJIS&encoding=base64&data="+encodeURIComponent(file.filedata), api_key, api_secret);
	sys.log("JapanNet: Data pushed to server, result: "+res);

	//tab.interact();

	// Logout
	tab.document().eval("nbLogout()");
	tab.waitFinish(500);
} catch(e) {
	sys.log("Exception: "+e);
	tab.interact();
	// Logout
	tab.document().eval("nbLogout()");
	tab.waitFinish(500);
}
