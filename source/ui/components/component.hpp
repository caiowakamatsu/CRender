//
// Created by Caio W on 12/2/22.
//

#ifndef CREBON_COMPONENT_HPP
#define CREBON_COMPONENT_HPP

namespace cr::component {
	template <typename T>
	class interface {
	private:
		typename T::Component _impl;

	public:
		interface() = default;

		[[nodiscard]] typename T::Options display(typename T::DisplayContents contents) const {
			return _impl.display(contents);
		}
	};
}

#endif //CREBON_COMPONENT_HPP
