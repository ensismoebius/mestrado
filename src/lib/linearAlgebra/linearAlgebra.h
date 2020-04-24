/**
 * @author André Furlan
 * @email ensismoebius@gmail.com
 * This whole project are under GPLv3, for
 * more information read the license file
 *
 * 29 de mar de 2020
 *
 */
#ifndef SRC_LIB_LINEARALGEBRA_LINEARALGEBRA_H_
#define SRC_LIB_LINEARALGEBRA_LINEARALGEBRA_H_
#include <vector>

namespace linearAlgebra {
	/**
	 * Function that return dot product of two vectors.
	 * @param a
	 * @param b
	 * @return
	 */
	long double dotProduct(std::vector<double> a, std::vector<double> b);

	/**
	 * Create an orthogonal vector given another one
	 * @param originalVector
	 * @param vectorSize
	 * @return
	 */
	double* calcOrthogonalVector(double *originalVector, int vectorSize);

	/**
	 * Given a vector calculates the corresponding orthogonal vector
	 * @param vector - source vector
	 * @return orthogonal vector
	 */
	std::vector<double> calcOrthogonalVector(std::vector<double> &vector);

	/**
	 * Normalize the vector
	 * @param signal
	 * @param signalLength
	 */
	void normalizeVector(double *signal, int signalLength);

	/**
	 * Normalize the vector
	 * @param signal
	 */
	void normalizeVector(std::vector<double> &signal);

	/**
	 * Convolute a signal with a filter (kernel)
	 * @param data
	 * @param dataLength
	 * @param kernel
	 * @param kernelSize
	 * @return
	 */
	bool convolution(double *data, int dataLength, double *kernel, int kernelSize);

	/**
	 * Perform a DCT on vector
	 * @param vector
	 * @param vectorLength
	 */
	void discreteCosineTransform(double *vector, long vectorLength);

	/**
	 * Perform a DCT on vector
	 * @param vector
	 */
	void discreteCosineTransform(std::vector<double> vector);
}
#endif /* SRC_LIB_LINEARALGEBRA_LINEARALGEBRA_H_ */
