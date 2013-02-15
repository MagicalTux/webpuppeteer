sys.include("config.js");

var tab = sys.newTab();

// login
sys.log("Technocash: Logging in...");
tab.load("https://secure.technocash.com.au/billfold/");
tab.document().getElementsByName("Username")[0].setAttribute("value", technocash_billfold);
tab.document().getElementsByName("Password")[0].setAttribute("value", technocash_password);
tab.document().getElementsByName("CheckLogin")[0].click();
tab.waitFinish();

while(true) {
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/withdraw_pull", "bank=09bab99a-8408-4154-9cbe-6a04618b0e14", api_key, api_secret);
	sys.log(res);
	res = JSON.parse(res);
	if (res.result != "success") {
		sys.log(JSON.stringify(res));
		sys.abort();
	}

	var data = res["return"].data;
	var meta = res["return"].meta;

	if (!meta) break;

	sys.log("Technocash: Preparing to send...");
	tab.document().findAllContaining("Transfer_Funds")[0].click();
	tab.waitFinish();
	tab.document().findAllContaining("Australian \"To Bank\"")[0].click();
	tab.waitFinish();
	var sel = tab.document().getElementsByName("bankaccount")[0].firstChild();
	var key = meta.bsb.replace("-","")+"/"+meta.account_number;

	while(sel) {
		if (key == sel.textContent().trim().substr(0,16)) {
			break;
		}
		sel = sel.nextSibling();
	}

	if (!sel) {
		sys.log("Technocash: Target account not found, creating...");
		tab.document().findAllContaining("Add new Payee Account")[0].click();
		tab.waitFinish();
		tab.document().findAllContaining("Enter  Australian Bank details")[0].click();
		tab.waitFinish();
		tab.document().getElementsByName("NickName")[0].setAttribute("value", key);
		tab.document().getElementsByName("AccountName")[0].setAttribute("value", meta.name);
		tab.document().getElementsByName("AccountBSB")[0].setAttribute("value", meta.bsb.replace("-",""));
		tab.document().getElementsByName("AccountNumber")[0].setAttribute("value", meta.account_number);
		tab.document().getElementsByName("FirstSub")[0].click();
		tab.waitFinish();
		if (tab.document().getElementsByName("AccountBSB").length)
			tab.document().getElementsByName("AccountBSB")[0].setAttribute("value", meta.bsb.replace("-",""));
		tab.document().getElementsByName("AccountNumber")[0].setAttribute("value", meta.account_number);
		tab.document().getElementsByName("SecondSub")[0].click();
		tab.waitFinish();
		sys.log("Technocash: Account created, retrying send");
		continue; // retry
	}
	var sel = sel.attribute("value");
	sys.log("Technocash: Found target account, id="+sel);
	tab.document().getElementsByName("bankaccount")[0].eval("this.value="+JSON.stringify(sel));
	tab.document().getElementsByName("Amount")[0].setAttribute("value", data.Amount/100);
	tab.document().getElementsByName("Description")[0].setAttribute("value", data.Money_Bank_Withdraw__);
	tab.document().getElementsByName("Reference")[0].setAttribute("value", "MTGOX A"+data.Index);
	tab.document().getElementsByName("SubmitStep1")[0].click();
	tab.waitFinish();
	tab.document().getElementsByName("spend_pwd")[0].setAttribute("value", technocash_send);
	tab.document().getElementsByName("ConfirmTransfer")[0].click();
	tab.waitFinish();

	var tx_success = {};
	tx_success.trx = res["return"];
	tx_success.files = {};
	tx_success.files["receipt.pdf"] = tab.printBase64();
	sys.signedPost("https://mtgox.com/api/1/generic/bank/withdraw_pull_res", "bank=09bab99a-8408-4154-9cbe-6a04618b0e14&json="+encodeURIComponent(JSON.stringify(tx_success)), api_key, api_secret);
}

//tab.interact();

sys.log("Technocash: Logging out...");
tab.document().findAllContaining("Logout")[0].click();
tab.waitFinish();

