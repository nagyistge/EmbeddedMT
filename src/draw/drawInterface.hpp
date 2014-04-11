/*
 * drawInterface.hpp
 *
 *  Created on: Apr 8, 2014
 *      Author: cv
 */

#ifndef DRAWINTERFACE_HPP_
#define DRAWINTERFACE_HPP_

#include "cm/global.hpp"

namespace Draw {
class DrawInterface {
public:
	virtual GBL::CmRetCode_t draw(const GBL::Image_t& img1, const GBL::Image_t& img2, std::vector<GBL::Match_t> good_matches,
			std::vector<GBL::KeyPoint_t> keypoints1, std::vector<GBL::KeyPoint_t> keypoints2) const = 0;
};
}

#endif /* DRAWINTERFACE_HPP_ */
