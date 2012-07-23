// poland
sys.include("lib/centrum24.js");
sys.include("config.js");

var b = new centrum24();
b.login(centrum_nik, centrum_password);
b.interact();
b.logout();

