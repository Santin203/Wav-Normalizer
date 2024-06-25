/* File with the prototypes of the C functions */
/* By Santiago Jiménez */

// Function to calculate the average volume of a block of audio samples
float calculateAverageVolume(float* samples, int blockSize);

// Function to find the maximum volume of a block of audio samples
float findReferenceVolume(float* samples, int blockSize);

// Function to compute the amplification factors for each block
void computeAmplificationFactors(float* blockAverageVolumes, float referenceVolume, float* amplificationFactors, int numBlocks);

// Function to apply amplification factor to a block of audio samples
void applyAmplificationFactor(float* samples, float amplificationFactor, int blockSize);