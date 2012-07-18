# WebPuppeteer

Because I hate people, I made sure the project's name was easy enough to typo.

This project is about allowing a simple ECMA-compliant script to call pages in a browser environement, and mess with said pages as it pleases. Other actions such as loading HTTP content directly and separately are also eventually possible.

The goal is to ease automation of access on banks and such way more than what would be considered socially acceptable. The whole thing ain't finished yet, but will allow clicking links just by their visible text, clicking on arbitrary locations of pages, automatically exporting whole tables as CSV, and way more.

# Headless?

Running headless is easy, just use xvfb.

To have xvfb on Gentoo:

- x11-base/xorg-server minimal

# Tell me what that scripts can do

## WebPuppeteerSys

Initially you have access to this object in "sys" global (also aliased as "console" so you can type console.log(...) without any issue). This object has the following methods.

### log(message)

Display a message on the console.

```javascript
sys.log("Ready!");
```

### sleep(msec)

A bold name for a bold function. It basically waits for the specified amount of time (for example if a page contains some JS code that takes a bit of time to be ready) specified in milliseconds.

For example to wait one second:
```javascript
sys.sleep(1000);
```

### newTab()

Opens a new tab and returns a new WebPuppeteerTab object.

### get(url)

Get data from url, returns it.

```javascript
var next = JSON.parse(sys.get("http://localhost/next.php"));
```

## WebPuppeteerTab

### browse(url)

Browse to given url, waits until url is fully loaded.

### screenshot(filename)

Take a screenshot of the current page at current resolution.

### fullshot(filename)

Take a full-size screenshot of the page, even if it extends over the current resolution.

### print(filename)

Outputs current page as PDF.

### eval(javascript)

Execute javascript code in the page.

### findFirst(selectorQuery)

Locate first element matching the CSS2 selector passed as argument.

### findAll(selectorQuery)

Locate all elements matching the CSS2 selector passed as argument.

### document()

Returns the document element.

### treeDump()

Dump webkit's internal rendering tree.

### interact()

Opens the page in a window, and give the user opportunity to interact with the page.

