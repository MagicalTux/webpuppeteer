#include "TimeoutTrigger.hpp"

TimeoutTrigger::TimeoutTrigger(int timeout) {
	t.setInterval(timeout);
	t.setSingleShot(true);
	connect(&t, SIGNAL(timeout()), this, SLOT(timer_timeout()));
	t.start();
}

void TimeoutTrigger::timer_timeout() {
	if (in_op) return;
	timeout();
}

void TimeoutTrigger::start() {
	in_op = true;
	t.stop();
}

void TimeoutTrigger::end() {
	in_op = false;
	t.start();
}

