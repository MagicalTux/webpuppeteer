#include "WebPuppeteerTab.hpp"
#include "WebPuppeteer.hpp"

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *parent): QObject(parent) {
	// void
}

void WebPuppeteerTab::browse(const QString &url) {
	qDebug("browse to: %s", qPrintable(url));
}

