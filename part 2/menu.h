#pragma once

#ifndef MENU_H
#define MENU_H

#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include "incontrol.h"

class Item;
class Menu;
class OneActionMenu;
size_t select_option();
void menu_process(const Menu&);

class Item {
public:
	friend class Menu;
	Item(const std::string&, const std::function<void()>&);
private:
	std::string name;
	std::function<void()> action;
};

class Menu {
public:
	Menu(const std::string&, const std::string&, const std::vector<Item>&);
	void display(const std::string&) const;
	bool execute_request(const int&) const;
protected:
	std::string name;
	std::string close_option;
	std::vector<Item> v_items;
};

#endif // !MENU
