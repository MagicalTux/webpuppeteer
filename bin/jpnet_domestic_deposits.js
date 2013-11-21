// JapanNet Bank - BA Plus
sys.include("lib/jpnet.js");
sys.include("config.js");

var bank = new jpnet();
bank.login(jpnet_account, jpnet_login, jpnet_password);

var data = bank.getTodayDeposits();

if (data) {
	var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/push_csv", "bank=38e619ac-7b9a-49a8-8e70-3628c980d1f0&charset=SJIS&encoding=base64&data="+encodeURIComponent(data), api_key, api_secret);
	sys.log("JapanNet: Data pushed to server, result: "+res);
}

bank.logout();

