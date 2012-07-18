# WebPuppeteer

Because I hate people, I made sure the project's name was easy enough to typo.

This project is about allowing a simple ECMA-compliant script to call pages in a browser environement, and mess with said pages as it pleases. Other actions such as loading HTTP content directly and separately are also eventually possible.

The goal is to ease automation of access on banks and such way more than what would be considered socially acceptable. The whole thing ain't finished yet, but will allow clicking links just by their visible text, clicking on arbitrary locations of pages, automatically exporting whole tables as CSV, and way more.

# Headless?

Running headless is easy, just use xvfb.

To have xvfb on Gentoo:

- x11-base/xorg-server minimal

# Tell me what that JS can do

## sys

Initially you have access to a "sys" object (also aliased as "console" so you can type console.log(...) without any issue). This object has the following methods.

### log(message)

Display a message on the console.

```javascript
sys.log("Ready!");
```

### sleep(msec)

### newTab()

