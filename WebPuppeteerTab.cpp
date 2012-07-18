#include "WebPuppeteerTab.hpp"
#include "WebPuppeteerWebElement.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QWebFrame>
#include <QPainter>
#include <QPrinter>

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
	page = new QWebPage(this);

	// define standard values
	page->setViewportSize(QSize(1024,768));

	// disable scrollbars, not as anyone is going to use them anyway
	page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
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

bool WebPuppeteerTab::screenshot(const QString &filename) {
	QImage img(page->viewportSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	page->mainFrame()->render(&paint);
	paint.end();
	return img.save(filename);
}

bool WebPuppeteerTab::fullshot(const QString &filename) {
	QImage img(page->mainFrame()->contentsSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	page->mainFrame()->render(&paint);
	paint.end();
	return img.save(filename);
}

bool WebPuppeteerTab::print(const QString &filename) {
	QPrinter print;
	print.setOutputFileName(filename);
	print.setOutputFormat(QPrinter::PdfFormat);
	print.setPaperSize(QPrinter::A4);
	print.setPageMargins(0, 0, 0, 0, QPrinter::Inch);

	// we know our page is 72dpi, how many dpi do we need on the printer to fit it ?
	int dpi = (page->mainFrame()->contentsSize().width() * print.paperSize(QPrinter::Inch).width() / 72.0) * 1.06;
	print.setResolution(dpi);
	QPainter print_p(&print);
	page->mainFrame()->render(&print_p);
	print_p.end();
	return true;
}

QScriptValue WebPuppeteerTab::eval(const QString &js) {
	return parent->engine().newVariant(page->mainFrame()->evaluateJavaScript(js));
}

QScriptValue WebPuppeteerTab::findFirst(const QString &selector) {
	QWebElement el = page->mainFrame()->findFirstElement(selector);
	if (el.isNull()) return parent->engine().nullValue();
	return parent->engine().newQObject(new WebPuppeteerWebElement(el));
}

QString WebPuppeteerTab::treeDump() {
	return page->mainFrame()->renderTreeDump();
}

