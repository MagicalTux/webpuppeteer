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
		return this.getDepositsCallback(function() { tab.document().findFirst("form").eval("ToHonjitsu(this)"); tab.waitFinish(500); });
	};

	this.getYesterdayDeposits = function() {
		sys.log("JapanNet: Getting history for yesterday...");
		return this.getDepositsCallback(function() { tab.document().findFirst("form").eval("ToZenjitsu(this)"); tab.waitFinish(500); });
	};

	this.getDayDeposits = function(d_year, d_month, d_day) {
		sys.log("JapanNet: Getting history for given date...");
		var cb = function() {
			this.tab.document().GetElementsByName("ShokaiStartDateNen")[0].setAttribute("value", d_year);
			this.tab.document().GetElementsByName("ShokaiStartDateTsuki")[0].setAttribute("value", d_month);
			this.tab.document().GetElementsByName("ShokaiStartDateHi")[0].setAttribute("value", d_day);

			this.tab.document().GetElementsByName("ShokaiEndDateNen")[0].setAttribute("value", d_year);
			this.tab.document().GetElementsByName("ShokaiEndDateTsuki")[0].setAttribute("value", d_month);
			this.tab.document().GetElementsByName("ShokaiEndDateHi")[0].setAttribute("value", d_day);

			tab.document().findFirst("form").eval("ToKikan(this)");
			tab.waitFinish(500);
		};
		return this.getDepositsCallback(cb);
	};

	this.getDepositsCallback = function(cb) {
		try {
			tab.document().eval("toOthers()"); // account activity/balance/etc
			tab.waitFinish(500);
			tab.document().eval("mySubmit('202')"); // access movements list
			tab.waitFinish(500);
			tab.document().findFirst("input[type=button]").click();
			tab.waitFinish(500);

			cb(); // this will select the right period

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

