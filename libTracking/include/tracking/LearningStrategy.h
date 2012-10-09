/*
 * LearningStrategy.h
 *
 *  Created on: 21.09.2012
 *      Author: poschmann
 */

#ifndef LEARNINGSTRATEGY_H_
#define LEARNINGSTRATEGY_H_

#include <vector>
#include "tracking/Sample.h"

class FdImage;

namespace tracking {

class LearningMeasurementModel;

/**
 * Strategy for updating learning measurement models given the data of a condensation tracker.
 */
class LearningStrategy {
public:

	virtual ~LearningStrategy() {}

	/**
	 * Updates the measurement model without knowing the position of the tracked object.
	 *
	 * @param[in] model The learning measurement model.
	 * @param[in] image The image.
	 * @param[in] samples The current weighted samples of the condensation tracking.
	 */
	virtual void update(LearningMeasurementModel& model, FdImage* image, const std::vector<Sample>& samples) = 0;

	/**
	 * Updates the measurement model given the position of the tracked object.
	 *
	 * @param[in] model The learning measurement model.
	 * @param[in] image The image.
	 * @param[in] samples The current weighted samples of the condensation tracking.
	 * @param[in] position The position of the tracked object.
	 */
	virtual void update(LearningMeasurementModel& model, FdImage* image,
			const std::vector<Sample>& samples, const Sample& position) = 0;
};

} /* namespace tracking */
#endif /* LEARNINGSTRATEGY_H_ */