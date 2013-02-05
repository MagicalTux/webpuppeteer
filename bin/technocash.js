sys.include("config.js");

var tab = sys.newTab();

// login
sys.log("Technocash: Logging in...");
tab.load("https://secure.technocash.com.au/billfold/");
tab.document().getElementsByName("Username")[0].setAttribute("value", technocash_billfold);
tab.document().getElementsByName("Password")[0].setAttribute("value", technocash_password);
tab.document().getElementsByName("CheckLogin")[0].click();
tab.waitFinish();

// get AUD
sys.log("Technocash: Getting AUD history...");
tab.browse("bf_manage.cfm?action=drilldown&c=AUD");
tab.waitFinish();
tab.document().getElementsByName("Show_CSV")[0].eval("this.checked = true;");
tab.document().getElementsByName("subDateRange")[0].click();
tab.waitFinish();
// CVS
var csv = tab.document().getElementsByName("CVS")[0].eval("this.value");
//sys.log(csv);

sys.log("Technocash: pushing data to server");
var res = sys.signedPost("https://mtgox.com/api/1/generic/bank/push_csv", "bank=09bab99a-8408-4154-9cbe-6a04618b0e14&data="+encodeURIComponent(csv), api_key, api_secret);
sys.log("Technocash: "+res);
//res = JSON.parse(res);

//tab.interact();

sys.log("Technocash: Logging out...");
tab.document().findAllContaining("Logout")[0].click();
tab.waitFinish();

