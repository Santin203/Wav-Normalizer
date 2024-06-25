/* File with functions used in the main program */
/* Main program is onlyc.c */
/* By Santiago Jiménez */

#include "cfunctions_headers.h"
#include <math.h>

// Function to calculate the average volume of a block of audio samples
float calculateAverageVolume(float* samples, int blockSize) {
	float sum = 0.0;

    // Sum the absolute values of the samples in the block
	for (int i = 0; i < blockSize; i++) {
		sum += fabs(samples[i]);

		// If we reach the end of the block, and the last sample is -1.0, then we have reached the end of the audio file
		if (samples[i] == -100.0) {
			sum -= fabs(samples[i]);

			// Fill the rest of the block with a value of 0.3
			for (int j = 0; j < blockSize; j++) {
				samples[j] = 0.3;
			}
		}
	}

    // Return the average volume of the block
	return sum / blockSize;
}

// Function to find the maximum volume of a block of audio samples
float findReferenceVolume(float* samples, int blockSize) {
	float max = 0.0;

    // Find the maximum absolute value of the samples in the block
	for (int i = 0; i < blockSize; i++) {
		if (fabs(samples[i]) > max) {
			max = fabs(samples[i]);
		}
	}

    // Return the maximum volume of the block
	return max;
}

// Function to compute the amplification factors for each block
void computeAmplificationFactors(float* blockAverageVolumes, float referenceVolume, float* amplificationFactors, int numBlocks) {
	
    // Compute the amplification factor for each block
    for (int i = 0; i < numBlocks; i++) {
		amplificationFactors[i] = referenceVolume / blockAverageVolumes[i];
	}
}

// Function to apply the amplification factor to each block
void applyAmplificationFactor(float* samples, float amplificationFactor, int blockSize) {

    // Apply the amplification factor to each sample in the block
	for (int i = 0; i < blockSize; i++) {
		samples[i] *= amplificationFactor;
	}
}