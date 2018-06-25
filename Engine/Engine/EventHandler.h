#pragma once

class EventHandlerBase {
public:
	virtual void execute() = 0;
};

template <typename Class>
class EventHandler : public EventHandlerBase {

	typedef void (Class::*_fptr)(void);

public:

	Class * object;
	_fptr function;

	EventHandler(Class *obj, _fptr func) {
		object = obj;
		function = func;
	}

	void execute() {
		(object->*function)();
	}
};