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

Browse to given url (paths relative to the current page are acceptable), waits until url is fully loaded.

### post(url, post data, content type)

Posts data to url as if submitted by browser. content type is application/x-www-form-urlencoded by default.

### wait()

Wait for page load operations to finish (use for example after clicking a link or a button).

### waitFinish()

Wait for page load operations to finish, with a bit more time in case something loads after page load ends (redirect, etc).

### getNewWindow()

In case a script/etc opened a new window, gets that window (returns a WebPuppeteerTab object).

### back()

Go back one page.

### reload(force no cache)

Reload page. If force no cache is true, local cache will be ignored.

### screenshot(filename)

Take a screenshot of the current page at current resolution.

### fullshot(filename)

Take a full-size screenshot of the page, even if it extends over the current resolution.

### print(filename)

Outputs current page as PDF.

### printBase64()

Same as print, but returns base64 encoded PDF instead of writing to file.

### saveNetwork(filename)

Starts saving all network activity to filename, including loaded URLs and returned data, allowing for later reproduction of the same page load.

### eval(javascript)

Execute javascript code in the page.

### get(url)

Get url in the context of the web page, and return contents.

### overrideUserAgent(ua)

Sets user agent to ua

### setDefaultCharset(charset)

Change default charset in page loading.

### getDownloadedFile()

Returns latest downloaded file as an array with "filename" (name of file) and "filedata" (base64 encoded downloaded data). Not suitable for large files.

### document()

Returns the document element.

### treeDump()

Dump webkit's internal rendering tree.

### getHtml()

Return page html as string.

### type(text)

Cause text to be inputted as if typed by someone on a keyboard.

### typeEnter()

Cause keypress of enter key.

### typeTab()

Cause keypress of tab key.

### interact()

Opens the page in a window, and give the user opportunity to interact with the page.

## WebPuppeteerWebElement

This class represents one element inside the page. It acts in a similar way to DOM.

### attribute(name)

Get attribute value.

### setAttribute(name, value)

Set a given attribute.

### hasAttribute(name)

Check if has an attribute.

### xml()

Return element code as xml.

### textContent()

Return element text content.

### eval(js)

Eval javascript in context of element.

### click()

Cause click event to happen on element.

### onblur()

Cause blur event to happen on element.

### onchange()

Cause change event to happen on element.

### setStyleProperty(name, value)

Set css style property.

### getComputedStyle(name)

Get css property name as computed.

### tagName()

Return element's tag name.

### find(conditions)

Find all elements matching conditions set. For example: find({tagname:"a", id:"test"})

### findFirst(selectorQuery)

Locate first element matching the CSS2 selector passed as argument.

### findAll(selectorQuery)

Locate all elements matching the CSS2 selector passed as argument.

### findAllContaining(text)

Locate all elements containing the given string.

### getElementById(id)

Same as DOM.

### getElementsByTagName(tag)

Same as DOM.

### getElementsByName(name)

Same as DOM.

### parentNode()

### firstChild()

### nextSibling()

### frameDocument(framename)

Get root document object for a given frame directly in the object's context.

### setFocus()

Set focus on given object (for example if input text, any input will happen there).

### hasFocus()

Returns true if given element has focus.
