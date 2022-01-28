#include "Emulator.h"
#include <cmath>
#include <functional>

class Game
{
public:

	bool ended(Node* n)
	{
		State* s = n->state;
		return (s->frame >= 10);
	}

	int score(Node* n)
	{
		std::cout << "Game::score" << std::endl;
		if (ended(n)) return 1;
		return 0;
	}

	void rollout(Emulator e)
	{
		std::cout << "Game::rollout" << std::endl;
		// ... fill in 
	}

	void perform(Node* n, Emulator& e)
	{
		std::cout << "Game::perform(" << (n->action.id) << ")" <<std::endl;
		std::cout << "Using emu @ " << &e << std::endl;
		Action act = n->action;
		std::string id = act.id;
		State* s = &(e.state);

		if (id == "Wait")
		{
			e.run();
		}

		if (id == "PressStart")
		{
			s->ram[0x6E] == 0x08;
			e.run();
			s->ram[0x6E] == 0x08;
			e.run();
		}

	}

	std::vector<Action> getActs(Node* n)
	{
		std::cout << "getActs()" << std::endl;
		std::vector<Action> acts;
		// wait one frame
		acts.push_back(Action("Wait"));
		// press start
		acts.push_back(Action("PressStart"));

		return acts;
	}

	void expand(Node* n)
	{
		std::cout << "expand()" << std::endl;
		std::vector<Action> acts = getActs(n);
		if (acts.empty())
			return;

		n->children = new std::vector<Node*>;

//		std::cout << "Starting loop" << std::endl;
		for (int i = 0; i < acts.size(); i++)
		{
			Action curAct = acts[i];
//			std::cout << "act 1: " << curAct.id << std::endl;
			Node* newKid = new Node(curAct.id);
			newKid->action = curAct;
			newKid->parent = n;
			n->children->push_back(newKid);
		}
	}

	bool endFlag = false;

};

class MCTS
{
public:

	void markTerminal(Node* n)
	{
		std::cout << "MCTS::markTeminal" << std::endl;
		n->terminal = true;
		if (n->parent == nullptr) return;
		(n->parent->terminalKids)++;
		if (n->parent->terminalKids == (n->parent->children)->size())
		{
			markTerminal(n->parent);
		}
	}

	Node* selection(Node* n)
	{
		std::cout << "MCTS::selection" << std::endl;
		Node* topChild = nullptr;
		float topScore = -9999;

		const int c = sqrt(2);
		int k = n->value / 2;

		std::vector<Node*> cs = *(n->children);

		for (int i = 0; i < cs.size(); i++)
		{
			int score;
			Node* kid = cs[i];
			if (kid->terminal)
				std::cout << "Skipping terminal kid... " << std::endl;
			else
			{
				if (kid->visits > 0)
					score = kid->value + c * sqrt((2 * log(n->visits)) / (kid->visits));
				else
					score = k + c * sqrt((2 * log(n->visits)) / ( (n->cn) + 1));
			
				if (score > topScore)
				{
					topChild = kid;
					topScore = score;
				}
			}
		}

		if (topChild == nullptr)
			std::cout << "Selection ERROR: node has no children";

		if (topChild->visits == 0)
			n->cn = n->cn + 1;

		std::cout << "Selected top child: " << topChild->name << std::endl;
		return topChild;
	}

	Node* search(Emulator e, Game g, int numItr, Node* root)
	{
		std::cout << "MCTS::search" << std::endl;
		root->name = "root";
		for (int i = 0; i < numItr; i++)
		{
			g.endFlag = false;

			std::cout << "iteration #" << i << std::endl;
			Node* curNode = root;
			if (curNode->terminal) break;
			curNode->visits++;
			while (curNode->state != nullptr)
			{
				if (curNode->children == nullptr)
				{
					std::cout << "curNode->children == nullptr" << std::endl;
					e.loadstate(curNode);
					g.expand(curNode);
					if (curNode->children == nullptr)
					{
						std::cout << "No children. skipping..." << std::endl;
						g.endFlag = true;
						break;
					}
				}
				curNode = selection(curNode);
				curNode->visits++;
			}

			if (!g.endFlag)
			{
//				std::cout << "Has not hit end flag, so we will perform..." << std::endl;
				e.loadstate(curNode->parent);
				std::cout << "Performing with emu @ " << &e << std::endl;
				g.perform(curNode, e);
				std::cout << "After performing, frame # is now: " << e.state.frame << std::endl;
				e.savestate(curNode);
				if (g.score(curNode) < e.bestScore)
					g.endFlag = true;
				g.endFlag = g.ended(curNode);
			}

			if (g.endFlag)
				markTerminal(curNode);
			else
				g.rollout(e);

			int result = g.score(curNode);
			if (result > e.bestScore)
				e.bestScore = result;

			if (g.endFlag)
			{
				std::cout << "Game end detected" << std::endl;
				std::cout << "Score: " << g.score(curNode);
				system("pause");
			}
			else
			{
				std::cout << "Game end not detected" << std::endl;
				std::cout << "frame #: " << (curNode->state)->frame << std::endl;
				system("pause");
			}

			while (curNode != nullptr)
			{
				if (result > curNode->value)
					curNode->value = result;
				curNode = curNode->parent;
			}


			
		}

		Node* topKid = nullptr;
		int topScore = -9999;

		std::vector<Node*> kids = *(root->children);

		for (int i = 0; i < kids.size(); i++)
		{
			Node* k = kids[i];
			if (k->value > topScore)
			{
				topKid = k;
				topScore = k->value;
			}
		}
			
		return topKid;
	}

	void simulate(Emulator e, Game g, int numTurn, int numItr)
	{
		Node* root = new Node("");
		e.savestate(root);
		for (int i = 1; i < numTurn; i++)
		{
			std::cout << "Starting move " << i << std::endl;
			root = search(e, g, numItr, root);
			if (root == nullptr)
			{
				std::cout << "Null node reached: breaking" << std::endl;
				return;
			}
			e.loadstate(root);
			root->parent = nullptr;
			std::cout << "Selected move: " << root->name << " (score = " << root->value << ")" << std::endl;
		}
	}
};

int main()
{
	Emulator nes;
	std::ifstream file("RotTK2.nes", std::ifstream::binary);
	file >> nes;

	Game g;
	MCTS mcts;

	mcts.simulate(nes, g, 100, 100);
}