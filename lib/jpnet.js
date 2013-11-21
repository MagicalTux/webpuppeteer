// JapanNet Bank access script

jpnet = function() {
	this.tab = sys.newTab();
	this.tab.load("https://login.japannetbank.co.jp/balogin_L.html");

	this.login = function(account, login, pass) {
		sys.log("JapanNet: Logging in...");

		this.tab.document().getElementsByName("CifNo")[0].setAttribute("value", jpnet_account);
		this.tab.document().getElementsByName("UserId")[0].setAttribute("value", jpnet_login);
		this.tab.document().getElementsByName("Pw")[0].setAttribute("value", jpnet_password);

		this.tab.document().getElementsByName("login")[0].click();
		this.tab.waitFinish(500);

		if (this.tab.document().getElementsByName("Pw")[0]) {
			sys.log("JapanNet: Did not properly logout, re-login required");
			this.tab.document().getElementsByName("CifNo")[0].setAttribute("value", jpnet_account);
			this.tab.document().getElementsByName("UserId")[0].setAttribute("value", jpnet_login);
			this.tab.document().getElementsByName("Pw")[0].setAttribute("value", jpnet_password);
			this.tab.document().findFirst("input[type=image]").click();
			this.tab.waitFinish(500);
		}
		return true;
	};

	this.getTodayDeposits = function() {
		sys.log("JapanNet: Getting history for today...");
		return this.getDepositsCallback("this.tab.document().findFirst(\"form\").eval(\"ToHonjitsu(this)\");");
	};

	this.getYesterdayDeposits = function() {
		sys.log("JapanNet: Getting history for yesterday...");
		return this.getDepositsCallback(function(doc) { doc.findFirst("form").eval("ToZenjitsu(this)"); });
	};

	this.getDayDeposits = function(d_year, d_month, d_day) {
		sys.log("JapanNet: Getting history for given date...");
		var cb = function(tab) {
			tab.document().GetElementsByName("ShokaiStartDateNen")[0].setAttribute("value", d_year);
			tab.document().GetElementsByName("ShokaiStartDateTsuki")[0].setAttribute("value", d_month);
			tab.document().GetElementsByName("ShokaiStartDateHi")[0].setAttribute("value", d_day);

			tab.document().GetElementsByName("ShokaiEndDateNen")[0].setAttribute("value", d_year);
			tab.document().GetElementsByName("ShokaiEndDateTsuki")[0].setAttribute("value", d_month);
			tab.document().GetElementsByName("ShokaiEndDateHi")[0].setAttribute("value", d_day);

			tab.document().findFirst("form").eval("ToKikan(this)");
		};
		return this.getDepositsCallback(cb);
	};

	this.getDepositsCallback = function(xpr) {
		try {
			this.tab.document().eval("toOthers()"); // account activity/balance/etc
			this.tab.waitFinish(500);
			this.tab.document().eval("mySubmit('202')"); // access movements list
			this.tab.waitFinish(500);
			this.tab.document().findFirst("input[type=button]").click();
			this.tab.waitFinish(500);

			eval(xpr);
//			cb(this.tab.document()); // this will select the right period
			this.tab.waitFinish(500);

			sys.log("JapanNet: Downloading...");
			this.tab.document().eval("CsvClick()"); // access movements list
			this.tab.waitFinish(500);

			var win = this.tab.getNewWindow();
			if (!win) throw "Got no window while expecting one";

			while(true) {
				var file = win.getDownloadedFile();
				if (!file) {
					sys.sleep(100);
					continue;
				}
				break;
			}

			return file.filedata;
		} catch(e) {
			sys.log("Exception: "+e);
		}
		return false;
	};

	this.logout = function() {
		// Logout
		this.tab.document().eval("nbLogout()");
		this.tab.waitFinish(500);
	};
};

