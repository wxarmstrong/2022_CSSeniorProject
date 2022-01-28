#ifndef ACTION_H
#define ACTION_H

class Action
{
public:
	Action(){}
	Action(std::string i)
	{
		id = i;
	}
	Action(std::string i, std::vector<std::string> p)
	{
		id = i;
		params = p;
	}
	std::string id;
	std::vector<std::string> params;
};

#endif ACTION_H