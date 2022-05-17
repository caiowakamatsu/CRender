//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_CONSOLE_HPP
#define CREBON_CONSOLE_HPP

#include <vector>
#include <string>

namespace cr::component {
	class console {
	public:
		struct Options {
		};

		struct DisplayContents {
			std::vector<std::string> *lines;
		};

		struct Component {
			[[nodiscard]] console::Options display(DisplayContents contents) const;
		};
	};
}

#endif //CREBON_CONSOLE_HPP
