#include "WebPuppeteerConsole.hpp"
#include "WebPuppeteer.hpp"

WebPuppeteerConsole::WebPuppeteerConsole(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
}

void WebPuppeteerConsole::log(QString msg) {
	qDebug("%s", qPrintable(msg));
}

