//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_SETTINGS_HPP
#define CREBON_SETTINGS_HPP

namespace cr::component {

	class settings {
	public:
		struct Options {

		};
		struct DisplayContents {

		};

		struct Component {
			[[nodiscard]] settings::Options display(DisplayContents contents) const;
		};
	};
}


#endif //CREBON_SETTINGS_HPP
