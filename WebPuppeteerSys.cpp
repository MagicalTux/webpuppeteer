#include "WebPuppeteerSys.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QTimer>

WebPuppeteerSys::WebPuppeteerSys(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
}

void WebPuppeteerSys::log(QString msg) {
	qDebug("%s", qPrintable(msg));
}

void WebPuppeteerSys::sleep(int msecs) {
	QEventLoop e;
	QTimer::singleShot(msecs, &e, SLOT(quit()));
	e.exec();
}

