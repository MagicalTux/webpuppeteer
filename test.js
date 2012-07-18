// JS script for WebPuppeteer
// it can do stuff!

console.log("hi, it's me... Waiting 500 ms");
sys.sleep(500);
console.log("finished!");

console.log("Loading google.com...");
tab = sys.newTab();
tab.browse("http://www.google.com/");
console.log("loaded, taking screenshot & saving as PDF!");

tab.eval("document.getElementsByName(\"q\")[0].value = \"Real men input their search here!\"");

console.log("current search value: "+tab.eval("document.getElementsByName(\"q\")[0].value"));

tab.screenshot("google.png");
tab.print("google.pdf");

