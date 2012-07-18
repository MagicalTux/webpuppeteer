// JS script for WebPuppeteer
// it can do stuff!

console.log("Loading google.com...");
tab = sys.newTab();
tab.browse("http://www.google.com/");
console.log("loaded, messing with the page");

tab.eval("document.getElementsByName(\"q\")[0].value = \"Real men input their search here!\"");
console.log("current search value: "+tab.eval("document.getElementsByName(\"q\")[0].value"));

// get all inputs
console.log("Listing all input tags:");
inputs = tab.findAll("input");
for(var i=0; i<inputs.length; i++) {
	console.log("input: "+inputs[i].xml());
}

console.log("taking screenshot");

tab.screenshot("google.png");
tab.print("google.pdf");

