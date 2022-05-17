//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_PREVIEW_HPP
#define CREBON_PREVIEW_HPP

#include <util/image.hpp>

#include <mutex>

namespace cr::component {
	struct preview {
	public:
		struct Options {
		};
		struct DisplayContents {
			cr::atomic_image *frame;
		};

		struct Component {
			[[nodiscard]] preview::Options display(DisplayContents contents) const;
		};
	};
}


#endif //CREBON_PREVIEW_HPP
