/**
 * @author André Furlan
 * @email ensismoebius@gmail.com
 * This whole project are under GPLv3, for
 * more information read the license file
 *
 * 12 de abr de 2020
 */

#ifndef SRC_LIB_WAVELET_WAVELETTRANSFORMRESULTS_CPP_
#define SRC_LIB_WAVELET_WAVELETTRANSFORMRESULTS_CPP_

#include <cmath>
#include <vector>
#include <stdexcept>

namespace wavelets {
	class WaveletTransformResults {

		public:
			/**
			 * If true the results comes from a
			 * packet wavelet transform
			 */
			bool packet = false;

			/**
			 * The levels of transformations
			 * we have done to the signal
			 */
			unsigned int levelsOfTransformation = 0;

			/**
			 * The transformed signal
			 */
			std::vector<double> transformedSignal;

			WaveletTransformResults(unsigned int maxItens = 0) {
				this->transformedSignal.resize(maxItens);
			}

			/**
			 * Extracts the aproximation and details from the wavelet transformation
			 * @param detailIndex -1: Return the whole transformed signal
			 * @param detailIndex 0: Extracts the aproximation
			 * @param detailIndex 1 or more: Extracts the corresponding detail
			 * @return Whole transformed signal, aproximation or details
			 */
			std::vector<double> getWaveletTransforms(int detailIndex = -1) {

				// User is requesting more details then we had produced
				if (detailIndex > (int) this->levelsOfTransformation) {
					throw std::runtime_error("There is not a transformations at this level");
				}

				// Returns the full transformed signal
				if (detailIndex == -1) return this->transformedSignal;

				// Creating the indexers witch will point to the start (sstrat)
				// and the end (send) of the signal we want
				unsigned send = 0;
				unsigned sstart = 0;

				// The conteiner of our response
				std::vector<double> levelTransformedSignal;

				// This value is used in the calculations of indexers
				int log = (int) std::log2(this->transformedSignal.size());

				// User is requesting just the aproximation
				if (detailIndex == 0) {
					// Aproximations always starts at index 0
					sstart = 0;
					// The size of aproximation depends on size of original
					// signal and the levels of transformations made
					send = std::pow(2, log - this->levelsOfTransformation);
				} else {
					// If the executions reaches this part the user are interested in details
					sstart = std::pow(2, log - detailIndex);
					send = std::pow(2, log - detailIndex + 1);
				}

				// Assembling the response
				for (unsigned int indexRange = sstart; indexRange < send; indexRange++) {
					levelTransformedSignal.push_back(this->transformedSignal.at(indexRange));
				}

				return levelTransformedSignal;
			}

			/**
			 * Extracts the values of a wavelet packet transformation
			 * differently from @getWaveletTransforms it DO NOT returns
			 * the details of transformation, otherwise, returns the
			 * generated chunks of the transformed signal
			 * @param partIndex : A value from 0 up to @getWaveletPacketAmountOfParts
			 * @return the requested chunk
			 */
			std::vector<double> getWaveletPacketTransforms(unsigned int partIndex) {

				// Checks if this is a wavelet transform
				if (!this->packet) {
					throw std::runtime_error("This is not a wavelet packet transfomed signal");
				}

				// The partIndex must not access non existent parts of the transformation
				if (this->getWaveletPacketAmountOfParts() - 1 < partIndex) {
					throw std::runtime_error("You are trying to access a non existent part of transformation");
				}

				// Calculate de size of the chuncks
				int chunkSize = this->transformedSignal.size() / this->getWaveletPacketAmountOfParts();

				// Get the ranges that must be returned
				int sstart = partIndex * chunkSize;
				int send = sstart + chunkSize;

				// Returns the data
				return std::vector<double>(this->transformedSignal.begin() + sstart, this->transformedSignal.begin() + send);
			}

			unsigned int getWaveletPacketAmountOfParts() {
				return std::pow(2, this->levelsOfTransformation);
			}
	};
}

#endif /* SRC_LIB_WAVELET_WAVELETTRANSFORMRESULTS_CPP_ */
