#include <istream>
#include <optional>
#include <string>
#include <unordered_map>

namespace lsystem {
	typedef std::unordered_map<char, std::string> ruleset;
	ruleset parse_rules(std::istream& rules);
	std::string iterate(const std::string& seed, const ruleset& rules, int count = 1);

	namespace gui {
		static const char* available_rule_names[] = {
			"",
			"Basic Tree",
		};
		static const char* available_rule_paths[] = {
			"",
			CGRA_SRCDIR "/res/rules/basic_tree.txt",
		};
		struct Data {
			std::string seed;
			std::optional<ruleset> rules;
			int interations;
			bool regrow;
		};
		void rules_window(struct Data& data);
		void growth_window(struct Data& data);
	}
}
