#include "WebPuppeteerConsole.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QTimer>

WebPuppeteerConsole::WebPuppeteerConsole(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
}

void WebPuppeteerConsole::log(QString msg) {
	qDebug("%s", qPrintable(msg));
}

void WebPuppeteerConsole::sleep(int msecs) {
	QEventLoop e;
	QTimer::singleShot(msecs, &e, SLOT(quit()));
	e.exec();
}

