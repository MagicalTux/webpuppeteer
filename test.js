// JS script for WebPuppeteer
// it can do stuff!

console.log("Loading google.com...");
tab = sys.newTab();
tab.browse("http://www.google.com/");
console.log("loaded, messing with the page");

tab.eval("document.getElementsByName(\"q\")[0].value = \"Real men input their search here!\"");

console.log("current search value (method 1): "+tab.eval("document.getElementsByName(\"q\")[0].value"));

console.log("input name=q: "+tab.findFirst("input[name=\"q\"]").xml());

console.log("taking screenshot");

tab.screenshot("google.png");
tab.print("google.pdf");

