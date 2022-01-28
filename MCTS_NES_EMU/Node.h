#ifndef NODE_H
#define NODE_H

#include <vector>
#include "Action.h"

class Node
{
public:
	Node(std::string n)
	{
		name = n;
		state = nullptr;
	}
	std::string name = "";
	std::string id = "";
	Action action;
	State* state = nullptr;
	Node* parent = nullptr;
	std::vector<Node*>* children = nullptr;
	float value = 0;
	bool terminal = false;
	int terminalKids = 0;
	int visits = 0;
	int cn = 0;
};

#endif NODE_H