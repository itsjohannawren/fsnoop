#include "dll.h"

#ifndef NULL
#define NULL (void *) 0
#endif

struct dll {
	struct dll * prev, * next;
};

int dllAppend (void * list, void * element) {
	struct dll * _list, * _element;
	_list = (struct dll *) list;
	_element = (struct dll *) element;

	if ((_list->prev == NULL) && (_list->next == NULL)) {
		_element->prev = _element->next = NULL;
		_list->prev = _list->next = _element;

	} else if ((_list->prev != NULL) && (_list->next != NULL)) {
		_element->prev = _list->prev;
		_element->next = NULL;
		_list->prev->next = _element;
		_list->prev = _element;

	} else {
		return (-1);
	}

	return (0);
}

int dllPrepend (void * list, void * element) {
	struct dll * _list, * _element;
	_list = (struct dll *) list;
	_element = (struct dll *) element;

	if ((_list->prev == NULL) && (_list->next == NULL)) {
		_element->prev = _element->next = NULL;
		_list->prev = _list->next = element;

	} else if ((_list->prev != NULL) && (_list->next != NULL)) {
		_element->prev = NULL;
		_element->next = _list->next;
		_list->next->prev = _element;
		_list->next = _element;

	} else {
		return (-1);
	}

	return (0);
}

int dllInsert (void * list, void * after, void * element) {
	struct dll * _list, * _after, * _element;
	_list = (struct dll *) list;
	_after = (struct dll *) after;
	_element = (struct dll *) element;

	if (_after->next == NULL) {
		_element->prev = _after;
		_element->next = NULL;
		_after->next = _element;

	} else {
		_element->prev = _after;
		_element->next = _after->next;
		_after->next->prev = _element;
		_after->next = _element;
	}

	return (0);
}

int dllRemove (void * list, void * element) {
	struct dll * _list, * _element;
	_list = (struct dll *) list;
	_element = (struct dll *) element;

	if ((_element->prev == NULL) && (_element->next == NULL)) {
		_list->prev = _list->next = NULL;

	} else if (_element->prev == NULL) {
		_list->next = _element->next;
		_element->next->prev = NULL;

	} else if (_element->next == NULL) {
		_list->prev = _element->prev;
		_element->prev->next = NULL;

	} else {
		_element->prev->next = _element->next;
		_element->next->prev = _element->prev;
	}

	return (0);
}

int dllRemoveFirst (void * list) {
	struct dll * _list;
	_list = (struct dll *) list;

	if ((_list->prev != NULL) && (_list->next != NULL)) {
		return (dllRemove (_list, _list->next));
	}

	return (0);
}

int dllRemoveLast (void * list) {
	struct dll * _list;
	_list = (struct dll *) list;

	if ((_list->prev != NULL) && (_list->next != NULL)) {
		return (dllRemove (_list, _list->prev));
	}

	return (0);
}
