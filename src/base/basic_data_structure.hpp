#ifndef BASE_DATA_STRUCTURE_HPP
#define BASE_DATA_STRUCTURE_HPP

//һЩ���������ݽṹ�����ڻ�����
#include<vector>
#include<iostream>
#include<random>

namespace coral {
	//����
	const int MX_LEVEL = 32;
	const double P_FACTOR = 0.25;

	template<typename T>
	struct SkipListNode {
		T val;
		std::vector<SkipListNode<T>*> forward;
		SkipListNode(T val, int maxLevel = MX_LEVEL) : val(val), forward(maxLevel, nullptr) {}
	};

	template<typename T>
	class SkipList {
	public:
		
		SkipList() : head(new SkipListNode<T>(-1)), level(0), dis(0, 1) {

		}

		bool search(T target) {
			SkipListNode<T>* cur = this->head;
			for (int i = level - 1; i >= 0; i--) {
				while (cur->forward[i] and cur->forward[i]->val < target)cur = cur->forward[i];
			}
			cur = cur->forward[0]; //��һ��Ԫ��
			return cur and (cur->val == target);
		}

		void add(T num) {
			std::vector<SkipListNode<T>*>update(MX_LEVEL, head);
			SkipListNode<T>* cur = this->head;
			for (int i = level - 1; i >= 0; i--) {
				while (cur->forward[i] and cur->forward[i]->val < num)cur = cur->forward[i];
				update[i] = cur;
			}
			int lv = randomLevel();
			level = std::max(level, lv);
			SkipListNode<T>* newNode = new SkipListNode<T>(num, lv);
			for (int i = 0; i < lv; i++) {
				newNode->forward[i] = update[i]->forward[i]; //�����½ڵ�Ľڵ���
				update[i]->forward[i] = newNode;
			}
		}

		bool erase(T num) {
			std::vector<SkipListNode<T>*>update(MX_LEVEL, nullptr);
			SkipListNode<T>* cur = this->head;
			for (int i = level - 1; i >= 0; i--) {
				while (cur->forward[i] and cur->forward[i]->val < num)cur = cur->forward[i];
				update[i] = cur;
			}
			cur = cur->forward[0];
			if (!cur or cur->val != num)return false; //û������ڵ�
			for (int i = 0; i < level; i++) {
				if (update[i]->forward[i] != cur)break;
				update[i]->forward[i] = cur->forward[i]; //����
			}
			delete cur;
			while (level > 1 and head->forward[level - 1] == nullptr)level--;
			return true;
		}

	private:

		int randomLevel() { //�������һ������
			int lv = 1;
			while (dis(gen) < P_FACTOR and lv < MX_LEVEL)lv++;
			return lv;
		}

		SkipListNode<T>* head;
		int level;
		std::mt19937 gen{ std::random_device{}() };
		std::uniform_real_distribution<double>dis;
	};

} //namespace coral

#endif //BASE_DATA_STRUCTURE_HPP