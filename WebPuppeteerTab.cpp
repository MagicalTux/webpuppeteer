#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QWebFrame>

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *parent): QObject(parent) {
	page = new QWebPage(this);

	// define standard values
	page->setViewportSize(QSize(1024,768));
}

bool WebPuppeteerTab::browse(const QString &url) {
	QEventLoop e;
	page->mainFrame()->load(QUrl(url));

	connect(page, SIGNAL(loadFinished(bool)), &e, SLOT(quit()));
	connect(page, SIGNAL(loadFinished(bool)), this, SLOT(setReturnBool(bool)));
	e.exec();

	return return_bool;
}

void WebPuppeteerTab::setReturnBool(bool r) {
	return_bool = r;
}

