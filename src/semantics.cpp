#include "pfederc/semantics.hpp"
using namespace pfederc;

std::vector<semantic_interface*> environment::search(const std::string& name) const noexcept {
	std::vector<semantic_interface*> result;
	auto& sems = this->m_semantics[pfederc::hash(name)];

	for (auto it = sems.begin(); it != sems.end(); ++it) {
		if ((*it)->name() == name)
			result.push_back(&**it);
	}

	if (parent()) {
		auto res_parent = parent()->search(name);
		for (auto it = res_parent.begin(); it != res_parent.end(); ++it)
			result.push_back(*it);
	}

	return std::move(result);
}

bool environment::add(std::unique_ptr<semantic_interface> &&semantic) noexcept {
	m_semantics[semantic->hash()].push_back(std::move(semantic));
	return true;
}
